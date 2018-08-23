
#include <iostream>
#include "parser.h"

#include "WordDetector.h"
using namespace std;

int main() {
#if 1
    const char* db_path ="udgerdb.dat";
    ParserDbdata parserDbdata(db_path);
    Parser parser(parserDbdata,10000);
    UaResult uaResult;
    string ua = "Mozilla/5.0 (Linux; Android 4.4.2; N9515 Build/KVT49L) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.81 Mobile Safari/537.36";
    parser.parseUa(uaResult,ua);
    std::cout<<uaResult.toString()<<std::endl;
    return 0;
#endif
#if 0
    WordDetector wordDetector;
    WordInfo i(1,"abc");
    wordDetector.addWord(1,"abc");
    hash_set<int>* a =wordDetector.findWords("abc");
    return 0;
#endif
}

