/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_lru.h
 * Author: liusheng
 *
 * Created on 2016年6月28日, 下午1:24
 */

#ifndef KV_LRU_H
#define KV_LRU_H


#include "../tool/kv_base.h"
#include "../conf/kv_conf.h"



namespace pingkv
{

class kv_lru: public kv_base
{
public:
    struct lru_node
    {
        char* k; int k_len;
        char* v; int v_len;
        int ref_count;
        lru_node* prev;
        lru_node* next;
    };
    struct lru_head
    {
        int node_count;
        lru_node* hot;
        lru_node* cold;
    };
private:
    llist* _tab;
    int _bucket_count;
    int _size;
    int _max_size;
    int _max_elem_size;
    lru_head _lru;
    
    int _set(const char* k, int k_len, const char* v, int v_len);
    int _get(char** v, int* v_len, const char* k, int k_len);
    int _get(lru_node** node, const char* k, int k_len);
    int _del(lru_node* n);
    int _ageout();
    
    int _init_lru_node(lru_node** node, const char* k, int k_len, const char* v, int v_len, lru_node* prev, lru_node* next);
    int _lru_hash(const char* data, int data_len, int mod);
    int _lru_insert(lru_node* n);
    int _lru_del(lru_node* n);
    
public:
    kv_lru();
    int init(int bucket_count, int max_size, int max_elem_size);
    virtual ~kv_lru();
    
    int size() const;
    int set(const char* k, int k_len, const char* v, int v_len);
    int get(char** v, int* v_len, const char* k, int k_len);
    
};





/*
 * TEST


#define DATA_COUNT BUFSIZ
int main(int argc, char** argv) 
{
    pingkv::kv_lru lru;
    lru.init(BUFSIZ, BUFSIZ, 1024);    
    int get = 0;
    int hit = 0;
    double rate = 0.0;
    int* num = new int[DATA_COUNT];
    for(int i=0; i<DATA_COUNT; i++) *(num + i) = i;

    int k_len;
    int v_len;
    char* k = new char[21]; 
    char* v = new char[21]; 
    char* vv = NULL;
    
    for(int i=0; i<BUFSIZ*1024; i++)
    {
        snprintf(k, 20, "%d", *(num + (rand()%(DATA_COUNT))));
        snprintf(v, 20, "%d", *(num + (rand()%(DATA_COUNT))));
        k_len = strlen(k);
        v_len = strlen(v);
        if(lru.set(k, k_len, v, v_len) == -1) {lru.log_err();exit(1);}
        
        snprintf(k, 20, "%d", *(num + (rand()%(DATA_COUNT))));
        if(lru.get(&vv, &v_len, k, k_len) == -1)  {lru.log_err();exit(1);}
        get ++;
        
        if(vv) 
        {
            hit ++;
            delete[] vv;
        }
        
        if(i%204800 == 0)
        {
            rate = (double)hit*100/(double)get;
            cout << lru.size() << "   " << get << "   " << hit << "   " << rate << endl;
        }
        
    }
    delete[] k;
    delete[] v;
    delete[] num;
}

 */





}
#endif /* KV_LRU_H */
















