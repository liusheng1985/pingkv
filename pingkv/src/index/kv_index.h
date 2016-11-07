/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_index.h
 * Author: liusheng
 *
 * Created on 2016年6月20日, 上午9:55
 */

#ifndef M_INDEX_H
#define M_INDEX_H
#include "../tool/kv_base.h"
#include "../dbf/data_file.h"
#include "../cache/kv_lru.h"
#include "../conf/kv_conf.h"


namespace pingkv
{
typedef short key_len_t;
typedef int   value_len_t;
class kv_index: public kv_base
{
public:
    #define KV_INDEX_NODE_LEN (sizeof(data_file::data_block) - sizeof(data_file::block_head) - sizeof(data_file::block_tail) - sizeof(data_file::record_index))

    struct k_info
    {
        int left_child_block_id;
        int block_id;
        short row_idx;
        key_len_t key_len;
    };
    
    struct node_head
    {
        int block_id;
        short record_count;
        int parent_block_id;
        int right_child_block_id;
    };
    

private:
    string _file_name;
    string _dbname;
    int _file_id;
    data_file _dbf;
    kv_lru _lru;
    
    
    
    /******************************************************************************************
     *              init function
     ******************************************************************************************/
    int _create();
    int _open(bool create);
    
    
    
    /******************************************************************************************
     *              record function
     ******************************************************************************************/
    // return key length
    int _get_key_from_record(char** key, char* record);
    int _cmp_key(const char* k1, const short len1, const char* k2, const int len2) const;
    
    /******************************************************************************************
     *              node function
     ******************************************************************************************/
    int _get_root(char** root);
    int _node_free_space(char* node);
    int _add_node(int* block_id, const int parent_block_id);
    int _save_node(const char* node);
    int _get_node(char** node, const block_id_t block_id);
    
    
    /******************************************************************************************
     *              k-v function
     ******************************************************************************************/
    int _set(const char* key, const key_len_t key_len, const char* val, const int val_len);
    // return value len
    int _get(char** val, const char*key, const key_len_t key_len);
    int _del(const char* key, const key_len_t key_len);
    
    int _find_pos(char** pos, const char* node, const char* key, const key_len_t key_len);
    int _find_key(block_id_t* block_id, row_idx_t* row_idx, const char* node, const char* key, const key_len_t key_len);
    int _set_key(const char* node, const char* key, const key_len_t key_len, const char* value, const value_len_t value_len);
    int _del_key(const char* node, const char* key, const key_len_t key_len);
    
    /******************************************************************************************
     *              cache function
     ******************************************************************************************/
    int _cache_get(char** val, const char* key, const key_len_t key_len);
    int _cache_insert(const char* key, const key_len_t key_len, const char* val, const value_len_t val_len);
    
public:
    virtual ~kv_index();
    int init(const string& dbname, int file_id, const string& file_name);
    int open_index(bool create);
    
    
    // return length of v
    value_len_t get(char** v, const char* k, const key_len_t k_len);
    int set(const char* k, const key_len_t k_len, const char* v, const value_len_t v_len);
    int del(char* k, key_len_t k_len);
};

}



/*
 TEST 

int main(int argc, char** argv) 
{
    pingkv::kv_index idx;
    idx.init("testdb", 1, "/tmp/aaa");
    if(idx.open_index(true) == -1)
    {
        idx.log_err();
        exit(1);
    }

    char* v = NULL;
    string sk;
    string sv;
    int re = 0;
    
    for(int i=0; i<10000; i++)
    {
        sk = pingkv::tool::itostr(i);
        sv = sk+"xxxxxxxxxxxxxx";
        
        
        
//        cout << v.to_string() << "  " << sv.length() << "  " << v.len() << endl;
        
        if(idx.set(sk.data(), sk.length(), sv.data(), sv.length()) == -1)
        {
            idx.log_err();
            exit(1);
        }
        sv.clear();
        if((re = idx.get(&v, sk.data(), sk.length())) == -1)
        {
            idx.log_err();
            exit(1);
        }
        cout << string(v, re) << endl;
        delete[] v;
        v = NULL;
//        cout << re << "  " << k.to_string() << "   " << v.to_string() << endl;
    }
} 
 */





#endif /* M_INDEX_H */

