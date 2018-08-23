//
// Created by wll on 2018/8/10.
//

#include <algorithm>
#include <hash_set>
#include <vector>
#include <limits>
#include <stdexcept>
#include <iostream>
#ifndef UDGER_WORDDECTOR_H
#define UDGER_WORDDECTOR_H

using __gnu_cxx::hash_set;
using std::vector;
using std::string;

using std::numeric_limits;
struct WordInfo
{
    int id;
    string word;
    WordInfo(int id,string word){
        this->id =id;
        this->word =word;
    }
    WordInfo(void){}
};

class WordDetector {
private:
    static const int ARRAY_DIMENSION = 'z' - 'a';
    static const int ARRAY_SIZE = (ARRAY_DIMENSION + 1) * (ARRAY_DIMENSION + 1);
    int minWordSize =(numeric_limits<int>::max)();
public:
    vector<WordInfo*> *wordarray =nullptr;
    WordDetector(){
        wordarray = new vector<WordInfo*>[ARRAY_SIZE];
    };
    ~WordDetector(){
        if(wordarray) {
            delete[] wordarray;
        }
    }
    void addWord(int id,string word)
    {
        if (word.length() < minWordSize) {
            minWordSize = word.length();
        }

        transform(word.begin(), word.end(), word.begin(), ::tolower);
        int index = (word[0] - 'a') * ARRAY_DIMENSION + word[1] - 'a';
        if (index >= 0 && index < ARRAY_SIZE) {
            WordInfo *info =new WordInfo(id, word);
            wordarray[index].push_back(info);
        } else {
           throw std::runtime_error("Index out of hashmap" + std::to_string(id) + " : "+ word);
        }
    }
    hash_set<int>* findWords(string text){
        hash_set<int> *ret =new hash_set<int>();
        transform(text.begin(), text.end(), text.begin(), ::tolower);
        const int dimension = 'z' - 'a';
        int len = text.length() - (minWordSize - 1);
        for(int i=0; i < len; i++) {
            const char c1 = text[i];
            const char c2 = text[i+1];
            if (c1 >= 'a' && c1 <= 'z' && c2 >= 'a' && c2 <= 'z') {
                const int index = (c1 - 'a') * dimension + c2 - 'a';
                vector<WordInfo*> infos = wordarray[index];
                size_t len = infos.size();
                if (0<len && &infos!= NULL) {
                    for (size_t j =0; j <len; j ++) {
                        WordInfo * d = infos[j];
                        if (0 ==text.compare(i,d->word.size(),d->word)) {
                            ret->insert(d->id);
                        }
                    }
                }
            }
        }
        return ret;
    }
};

#endif //UDGER_WORDDECTOR_H
