//
// Created by wll on 2018/8/9.
//
#ifndef UDGER_LRUCACHE_H
#define UDGER_LRUCACHE_H

#include <unordered_map>
using std::unordered_map;
template <typename K,typename V>
struct Node{
    K key;
    V value;
    Node<K,V>*prev = nullptr;
    Node<K,V>*next = nullptr;
};

template <typename K,typename T>
class LRUCache{
public:
    LRUCache(int &capacity);
    void put(K key,T value);
    T get(K key);
    int getCapacity(){return capacity;}//cache miss count
    Node<K,T>* getHead(){return head;};
    ~LRUCache();
private:
    int capacity =0;//cache miss count
    unordered_map<K,Node<K,T>* > hashmap_;//hash表
    Node<K,T>*head = nullptr;
    Node<K,T>*tail = nullptr;//头节点尾节点
};

template <typename K,typename T>
LRUCache<K,T>::LRUCache(int &capacity)//construct
{
    this->capacity =capacity;
}

template <typename K,typename T>
void LRUCache<K,T>::put(K key,T value)
{
    typename  unordered_map<K,Node<K,T>* >::iterator got= hashmap_.find (key);
    if (got == hashmap_.end()) {
        Node<K,T>* node = new Node<K,T>();
        node->value = value;
        node->key = key;
        node->next = head;
        node->prev = NULL;
        if (head != NULL) {
            head->prev = node;
        }
        if (tail == NULL) {
            tail = head;
        }
        head = node;
        hashmap_.insert(std::make_pair(key, node));
        if (hashmap_.size() > capacity) {
            hashmap_.erase(tail->key);
            tail = tail->prev;
            tail->next = NULL;
        }
    }
    else
        got->second->value = value;
}
template <typename K,typename T>
T LRUCache<K,T>::get(K key)
{
    typename  unordered_map<K,Node<K,T>* >::iterator got= hashmap_.find (key);
    if (got != hashmap_.end()) {
        Node<K,T>* node =got->second;
        if (this->head != node) {
            if (node->next != NULL) {
                node->next->prev =node->prev;
            } else {
                tail = node->prev;
            }
            node->prev->next = node->next;
            head->prev = node;
            node->next = head;
            node->prev = NULL;
            head = node;
        }
        return node->value;
    }
    return T();
}
template <typename K,typename T>
LRUCache<K,T>::~LRUCache() {
    this->hashmap_.clear();
    this->head = nullptr;
    this->tail = nullptr;
}
#endif //UDGER_LRUCACHE_H
