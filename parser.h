//
// Created by wll on 2018/8/8.
//
#include "CppSQLite3.h"
#include "SQLQuery.h"
#include "UaResult.h"
#include "WordDetector.h"
#include "LRUCache.h"
#include <regex>
#include <iostream>

#ifndef UDGER_PARSER_H
#define UDGER_PARSER_H
using std::regex;
using std::smatch;
using std::to_string;
struct IdRegString {
    int id;
    int wordId1;
    int wordId2;
    regex* pattern;
    ~IdRegString(){
        delete pattern;
    }
};
struct ClientInfo {
    int clientId;
    int classId;
};
class ParserDbdata{

private:
    WordDetector* clientWordDetector = nullptr;
    WordDetector* deviceWordDetector= nullptr;
    WordDetector* osWordDetector= nullptr;
    vector<IdRegString*> clientRegstringList;
    vector<IdRegString*> osRegstringList;
    vector<IdRegString*> deviceRegstringList;
    bool prepared =false;
    regex* PAT_UNPERLIZE = nullptr;
    const char * dbFileName;
    friend class Parser;

private:
    vector<IdRegString*> prepareRegexpStruct(CppSQLite3DB &db,string regexpTableName);
    WordDetector* createWordDetector(CppSQLite3DB &db, string regexTableName, string wordTableName);
    void addUsedWords(__gnu_cxx::hash_set<int> &usedWords, CppSQLite3DB &db, const string &regexTableName, const string wordIdColumn);

public:
    void prepare(CppSQLite3DB &db);
    ~ParserDbdata();
    ParserDbdata(const char* dbFileName){
        this->dbFileName =dbFileName;
        this->PAT_UNPERLIZE = new regex("^/?(.*?)/si$");
    }

};

class Parser
{
private:
    const string DB_FILENAME = "udgerdb_v3.dat";
    const string UDGER_UA_DEV_BRAND_LIST_URL = "https://udger.com/resources/ua-list/devices-brand-detail?brand=";
    bool osParserEnabled = true;
    bool deviceParserEnabled = true;
    bool deviceBrandParserEnabled = true;
    bool is_connect = false;
    SqlQuery sqlQuery;
    CppSQLite3DB db;
    ParserDbdata *parserDbdata= nullptr;
    smatch* lastSmatch =nullptr;
    LRUCache<string, UaResult*> *cache = nullptr;
    unordered_map<string,CppSQLite3Statement*> sQlite3StmtMap;
    unordered_map<string,regex*> regexMap;

private:
    void deviceDetector(string & uaString, UaResult *ret, ClientInfo &clientInfo);
    void osDetector(string & uaString, UaResult *ret, ClientInfo &clientInfo);
    void fetchOperatingSystem(CppSQLite3Query &rs, UaResult *ret);
    void fetchDeviceBrand(string & uaString, UaResult *ret);
    void fetchDevice(CppSQLite3Query &rs, UaResult *ret);
    void fetchUserAgent(CppSQLite3Query &rs, UaResult *ret);
    void patchVersions(UaResult *ret);
    void prepare();
    ClientInfo clientDetector(string & uaString, UaResult *ret);
    CppSQLite3Query getFirstRow(string &sql, std::initializer_list<string> params);
    regex* getPatternFromCache(string &reg_str);
    int findIdFromList(string & uaString, __gnu_cxx::hash_set<int>* foundClientWords,  vector<IdRegString*> list);

public:
    Parser(ParserDbdata &pDbdata):Parser(pDbdata, 10000){};
    Parser(ParserDbdata &pDbdata,int cacheCapacity);
    ~Parser();
    void parseUa(UaResult &uaResult,string & uaString);
};


#endif //UDGER_PARSER_H
