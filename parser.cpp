//
// Created by wll on 2018/8/8.
//
//
#include "parser.h"

void SplitString(const string& s, vector<string>& v, const string& c);

ParserDbdata::~ParserDbdata() {
    if(clientWordDetector){
        delete clientWordDetector;
    }
    if(deviceWordDetector){
        delete deviceWordDetector;
    }
    if(osWordDetector){
        delete osWordDetector;
    }
    if(&clientRegstringList){
        for (const auto& n :clientRegstringList) {
            delete n;
        }
        clientRegstringList.clear();
    }
    if(&osRegstringList){
        for (const auto& n :osRegstringList) {
            delete n;
        }
        osRegstringList.clear();
    }
    if(&deviceRegstringList){
        for (const auto& n :deviceRegstringList) {
            delete n;
        }
        deviceRegstringList.clear();
    }
    if(PAT_UNPERLIZE){
        delete PAT_UNPERLIZE;
    }
}

void ParserDbdata::prepare(CppSQLite3DB &db) {
    if (!prepared) {
        clientRegstringList = prepareRegexpStruct(db, "udger_client_regex");
        osRegstringList = prepareRegexpStruct(db, "udger_os_regex");
        deviceRegstringList = prepareRegexpStruct(db, "udger_deviceclass_regex");
        clientWordDetector = createWordDetector(db, "udger_client_regex", "udger_client_regex_words");
        deviceWordDetector = createWordDetector(db, "udger_deviceclass_regex", "udger_deviceclass_regex_words");
        osWordDetector = createWordDetector(db, "udger_os_regex", "udger_os_regex_words");
        prepared = true;

    }
}
vector<IdRegString*> ParserDbdata::prepareRegexpStruct(CppSQLite3DB &db, string regexpTableName) {
    vector<IdRegString*> ret;
    string sql= "SELECT rowid, regstring, word_id, word2_id FROM " + regexpTableName + " ORDER BY sequence";
    CppSQLite3Query q;
    try{
        q = db.execQuery(sql.data());
    }catch(CppSQLite3Exception e) {
        std::cerr << e.errorCode() << e.errorMessage() << std::endl;
    }
    while (!q.eof())
    {
        IdRegString *irs =new IdRegString();
        irs->id = q.getIntField("rowid");
        irs->wordId1 = q.getIntField("word_id");
        irs->wordId2 = q.getIntField("word2_id");
        string str = q.getStringField("regstring");
        smatch reg_result;
        if (regex_match(str,reg_result,*this->PAT_UNPERLIZE)) {
            str = reg_result[1];
        }

        irs->pattern =  new regex(str, regex::icase);;
        ret.push_back(irs);
        q.nextRow();
    }
    q.finalize();
    return ret;
}

WordDetector* ParserDbdata::createWordDetector(CppSQLite3DB &db, const string regexTableName, const string wordTableName){
     __gnu_cxx::hash_set<int> usedWords;
    addUsedWords(usedWords, db, regexTableName, "word_id");
    addUsedWords(usedWords, db, regexTableName, "word2_id");
    WordDetector *result = new WordDetector();
    CppSQLite3Query q = db.execQuery(("SELECT * FROM " + wordTableName).data());
    while (!q.eof())
    {
        int id =q.getIntField("id");
        if(usedWords.count(id)>0){
            string word = q.getStringField("word");
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            result->addWord(id,word);
        }
        q.nextRow();
    }
    q.finalize();
    return result;
}
void ParserDbdata::addUsedWords(__gnu_cxx::hash_set<int> &usedWords, CppSQLite3DB &db,const string &regexTableName,const string wordIdColumn){
    CppSQLite3Query q = db.execQuery(("SELECT " + wordIdColumn + " FROM " + regexTableName).data());
    while (!q.eof())
    {
        usedWords.insert(q.getIntField(wordIdColumn.data()));
        q.nextRow();
    }
    q.finalize();
}
Parser::Parser(ParserDbdata &pDbdata, int cacheCapacity) {
    this->parserDbdata =&pDbdata;
    if(cacheCapacity >0){
        cache = new LRUCache<string, UaResult*>(cacheCapacity);
    }
}
Parser::~Parser() {
    try
    {
        for (const auto& n :sQlite3StmtMap) {
            delete n.second;
        }
        sQlite3StmtMap.clear();
        if(&db != nullptr&& is_connect){
            db.close();
        }
        if(cache){
            delete cache;
        }
        for (const auto& reg :regexMap) {
            delete reg.second;
        }
        regexMap.clear();
        if(lastSmatch){
            delete lastSmatch;
        }

    }
    catch (...)
    {
    }
}
void Parser::parseUa(UaResult &uaResult,string & uaString)
{
    UaResult *ret = nullptr;
    if(NULL != cache->getHead()){
        ret = cache->get(uaString);
        if(NULL != ret){
            uaResult =*ret;
            return;
        }
    }
    UaResult temp(uaString);
    ret =&temp;
    this->prepare();
    ClientInfo clientInfo = clientDetector(uaString, ret);
    if (osParserEnabled) {
        osDetector(uaString, ret, clientInfo);
    }
    if (deviceParserEnabled) {
        deviceDetector(uaString, ret, clientInfo);
    }
    if (deviceBrandParserEnabled) {
        if (!ret->getosFamilyCode().empty()) {
            fetchDeviceBrand(uaString, ret);
        }
    }
    if ( NULL !=cache) {
        cache->put(uaString,ret);
    }
    uaResult =*ret;
}

void Parser::prepare() {
    if(!is_connect){
        db.open(this->parserDbdata->dbFileName);
        is_connect = true;
    }
    this->parserDbdata->prepare(db);
}

CppSQLite3Query Parser::getFirstRow(string &sql, std::initializer_list<string> params) {
    unordered_map<std::string,CppSQLite3Statement*>::const_iterator got = this->sQlite3StmtMap.find(sql);
    CppSQLite3Statement* sqLite3Stmt = nullptr;
    try{
        if ( got == sQlite3StmtMap.end() ){
            CppSQLite3Statement* tempStmt= new CppSQLite3Statement();
            *tempStmt= this->db.compileStatement(sql.data());
            sQlite3StmtMap.insert(make_pair(sql,tempStmt));
            sqLite3Stmt =tempStmt;
        }
        else{
            sqLite3Stmt =got->second;
            sqLite3Stmt->reset();
        }
        for (int i = 0; i < params.size(); i++) {
            const char * s=(*(params.begin()+i)).c_str();
            sqLite3Stmt->bind(i+1,s);
        }
    }catch(CppSQLite3Exception& e) {
        std::cerr << e.errorCode() << e.errorMessage() << std::endl;
    }
    return sqLite3Stmt->execQuery();
}

ClientInfo Parser::clientDetector(string &uaString, UaResult *ret) {
    ClientInfo clientInfo = {};
    CppSQLite3Query userAgentRs1 = getFirstRow(this->sqlQuery.SQL_CRAWLER,{uaString});
    if (NULL!=&userAgentRs1 && !userAgentRs1.eof()) {
        fetchUserAgent(userAgentRs1, ret);
        clientInfo.classId = 99;
        clientInfo.clientId = -1;
    }
    else {
        int rowid = findIdFromList(uaString, this->parserDbdata->clientWordDetector->findWords(uaString), this->parserDbdata->clientRegstringList);
        if (rowid != -1) {
            CppSQLite3Query userAgentRs2 = getFirstRow(this->sqlQuery.SQL_CLIENT,{std::to_string(rowid)});
            if (NULL!=&userAgentRs2 && !userAgentRs2.eof()) {
                fetchUserAgent(userAgentRs2, ret);
                clientInfo.classId = ret->getclassId();
                clientInfo.clientId = ret->getclientId();
                patchVersions(ret);
            }
        } else {
            ret->setuaClass("Unrecognized");
            ret->setuaClassCode("unrecognized");
        }
    }
    return clientInfo;
}

void Parser::osDetector(string & uaString, UaResult *ret, ClientInfo &clientInfo) {
    int rowid = findIdFromList(uaString, this->parserDbdata->osWordDetector->findWords(uaString),
                               this->parserDbdata->osRegstringList);
    if (rowid != -1) {
        CppSQLite3Query query = getFirstRow(this->sqlQuery.SQL_OS,{to_string(rowid)});
        if (NULL!=&query && !query.eof()) {
            fetchOperatingSystem(query, ret);
        }
    } else {
        if (&(clientInfo.clientId) != NULL && clientInfo.clientId != 0) {
            CppSQLite3Query query1 = getFirstRow(this->sqlQuery.SQL_CLIENT_OS,{to_string(clientInfo.clientId)});
            if (NULL!=&query1 && !query1.eof()) {
                fetchOperatingSystem(query1, ret);
            }
        }
    }
}

void Parser::deviceDetector(string & uaString, UaResult *ret, ClientInfo &clientInfo){
    int rowid = findIdFromList(uaString, this->parserDbdata->deviceWordDetector->findWords(uaString),
                               this->parserDbdata->deviceRegstringList);
    if (rowid != -1) {
        CppSQLite3Query query = getFirstRow(this->sqlQuery.SQL_DEVICE,{to_string(rowid)});
        if (NULL!=&query && !query.eof()) {
            this->fetchDevice(query, ret);
        }
    } else {
        if (&(clientInfo.clientId) != NULL && clientInfo.clientId != 0) {
            CppSQLite3Query query1 = getFirstRow(this->sqlQuery.SQL_CLIENT_CLASS,{to_string(clientInfo.classId)});
            if (NULL!=&query1 && !query1.eof()) {
                this-> fetchDevice(query1, ret);
            }
        }
    }
}



void Parser::fetchUserAgent(CppSQLite3Query &rs, UaResult *ret){
    ret->setclassId(rs.getIntField("class_id"));
    ret->setclientId(rs.getIntField("client_id"));
    ret->setcrawlerCategory(rs.getStringField("crawler_category"));
    ret->setcrawlerCategoryCode(rs.getStringField("crawler_category_code"));
    ret->setcrawlerLastSeen(rs.getStringField("crawler_last_seen"));
    ret->setcrawlerRespectRobotstxt(rs.getStringField("crawler_respect_robotstxt"));
    ret->setua(rs.getStringField("ua"));
    ret->setuaClass(rs.getStringField("ua_class"));
    ret->setuaClassCode(rs.getStringField("ua_class_code"));
    ret->setuaEngine(rs.getStringField("ua_engine"));
    ret->setuaFamily(rs.getStringField("ua_family"));
    ret->setuaFamilyCode(rs.getStringField("ua_family_code"));
    ret->setuaFamilyHomepage(rs.getStringField("ua_family_homepage"));
    ret->setuaFamilyIcon(rs.getStringField("ua_family_icon"));
    ret->setuaFamilyIconBig(rs.getStringField("ua_family_icon_big"));
    ret->setuaFamilyInfoUrl(rs.getStringField("ua_family_info_url"));
    ret->setuaFamilyVendor(rs.getStringField("ua_family_vendor"));
    ret->setuaFamilyVendorCode(rs.getStringField("ua_family_vendor_code"));
    ret->setuaFamilyVendorHomepage(rs.getStringField("ua_family_vendor_homepage"));
    ret->setuaUptodateCurrentVersion(rs.getStringField("ua_uptodate_current_version"));
    ret->setuaVersion(rs.getStringField("ua_version"));
    ret->setuaVersionMajor(rs.getStringField("ua_version_major"));
}
void Parser::fetchOperatingSystem(CppSQLite3Query &rs, UaResult *ret){
    ret->setosFamily(rs.getStringField("os_family"));
    ret->setos(rs.getStringField("os"));
    ret->setosCode(rs.getStringField("os_code"));
    ret->setosFamilyCode(rs.getStringField("os_family_code"));
    ret->setosFamilyVendorHomepage(rs.getStringField("os_family_vendor_homepage"));
    ret->setosFamilyVendor(rs.getStringField("os_family_vendor"));
    ret->setosFamilyVendorCode(rs.getStringField("os_family_vendor_code"));
    ret->setosHomePage(rs.getStringField("os_home_page"));
    ret->setosIcon(rs.getStringField("os_icon"));
    ret->setosIconBig(rs.getStringField("os_icon_big"));
    ret->setosInfoUrl(rs.getStringField("os_info_url"));
}
void Parser::fetchDevice(CppSQLite3Query &rs, UaResult *ret){
    ret->setdeviceClass(rs.getStringField("device_class"));
    ret->setdeviceClassCode(rs.getStringField("device_class_code"));
    ret->setdeviceClassIcon(rs.getStringField("device_class_icon"));
    ret->setdeviceClassIconBig(rs.getStringField("device_class_icon_big"));
    ret->setdeviceClassInfoUrl(rs.getStringField("device_class_info_url"));
}

void Parser::fetchDeviceBrand(string & uaString, UaResult *ret) {
    unordered_map<std::string,CppSQLite3Statement*>::const_iterator got = this->sQlite3StmtMap.find(this->sqlQuery.SQL_DEVICE_REGEX);
    CppSQLite3Statement* sqLite3Stmt = nullptr;
    if ( got == sQlite3StmtMap.end() ){
        CppSQLite3Statement* tempStmt= new CppSQLite3Statement();
        *tempStmt= db.compileStatement(this->sqlQuery.SQL_DEVICE_REGEX.data());
        sQlite3StmtMap.insert(make_pair(this->sqlQuery.SQL_DEVICE_REGEX,tempStmt));
        sqLite3Stmt =tempStmt;
    }
    else{
        sqLite3Stmt =got->second;
        sqLite3Stmt->reset();
    }
    sqLite3Stmt->bind(1,ret->getosFamilyCode().data());
    sqLite3Stmt->bind(2,ret->getosCode().data());
    CppSQLite3Query query =sqLite3Stmt->execQuery();
    if (&query !=NULL) {
        while (!query.eof()) {
            string devId = query.getStringField("id");
            string regex_str = query.getStringField("regstring");
            if (!devId.empty() && !regex_str.empty()) {
                regex* re = this->getPatternFromCache(regex_str);
                smatch sm;   // 存放string结果的容器
                string str(uaString);
                if (regex_search(str,sm,*re)) {
                    CppSQLite3Query devNameListRs= getFirstRow(sqlQuery.SQL_DEVICE_NAME_LIST, {devId, sm[1]});
                    if (NULL!=&query && !query.eof()) {
                        ret->setdeviceMarketname(devNameListRs.getStringField("marketname"));
                        ret->setdeviceBrand(devNameListRs.getStringField("brand"));
                        ret->setdeviceBrandCode(devNameListRs.getStringField("brand_code"));
                        ret->setdeviceBrandHomepage(devNameListRs.getStringField("brand_url"));
                        ret->setdeviceBrandIcon(devNameListRs.getStringField("icon"));
                        ret->setdeviceBrandIconBig(devNameListRs.getStringField("icon_big"));
                        ret->setdeviceBrandInfoUrl(UDGER_UA_DEV_BRAND_LIST_URL + devNameListRs.getStringField("brand_code"));
                        break;
                    }
                }
            }
            query.nextRow();
        }
    }
    sqLite3Stmt->finalize();
}
void Parser::patchVersions(UaResult *ret) {
    if (lastSmatch != NULL) {
        string version = "";
        if (lastSmatch->size() > 1) {
            version = *(lastSmatch->begin()+1).base();
            if (version.empty()) {
                version = "";
            }
        }
        ret->setuaVersion(version);
        vector<string> v;
        SplitString(version, v,".");
        if (v.size()> 0) {
            ret->setuaVersionMajor(v[0]);
        } else {
            ret->setuaVersionMajor("");
        }
        ret->setua((!ret->getua().empty() ? ret->getua() : "") + " " + version);
    } else {
        ret->setuaVersion("");
        ret->setuaVersionMajor("");
    }
}
regex* Parser::getPatternFromCache(string &reg_str) {
    unordered_map<std::string,regex*>::const_iterator got = this->regexMap.find(reg_str);
    regex * reg =nullptr;
    if (got == regexMap.end()) {
        smatch reg_result;
        if (regex_match(reg_str,reg_result,*this->parserDbdata->PAT_UNPERLIZE)) {
            reg_str = reg_result[1];
        }
        reg = new regex(reg_str,regex::icase);
        regexMap.insert(make_pair(reg_str, reg));
    }
    return reg;
}



int Parser::findIdFromList(string & uaString, __gnu_cxx::hash_set<int>* foundClientWords,  vector<IdRegString*> list) {
    lastSmatch =new smatch();
    for (IdRegString* irs : list) {
        if ((irs->wordId1 == 0 || foundClientWords->count(irs->wordId1)) &&
            (irs->wordId2 == 0 || foundClientWords->count(irs->wordId2))) {
            if (regex_search(uaString,*lastSmatch,*irs->pattern)) {
                return irs->id;
            }
        }
    }
    return -1;
}




void SplitString(const string& s, vector<string>& v, const string& c)
{
    if(s.empty()){
        return ;
    }
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}
