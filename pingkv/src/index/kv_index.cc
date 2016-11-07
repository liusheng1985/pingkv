/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_index.cc
 * Author: liusheng
 * 
 * Created on 2016年6月20日, 上午9:55
 */

#include "kv_index.h"

namespace pingkv
{

kv_index::~kv_index() {}

int kv_index::init(const string& dbname, int file_id, const string& file_name)
{
    _dbname = dbname;
    _file_id = file_id;
    _file_name = file_name;
    _module_name = "INDEX_";
    _module_name.append(tool::itostr(file_id));
    if(_lru.init(BUFSIZ, BUFSIZ*1024, 1024) == -1) goto err;
    
    return 0;
    err:
    char e[BUFSIZ];
    sprintf(e, "init index error: datafile %s\n", file_name.c_str());
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
    
}


int kv_index::open_index(bool create)
{
    if(this->_open(create) == -1) goto err;
    if(_dbf.flush_blocks() == -1) goto err;
    return 0;
    err:
    char e[BUFSIZ];
    sprintf(e, "open index error: datafile %s\n", _file_name.c_str());
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}


int kv_index::get(char** v, const char* k, const key_len_t k_len)
{
    char* val = NULL;
    
    int re = _cache_get(v, k, k_len);
    if(re == -1) goto err;
    if(re > 0) return re;
    
    if((re = _get(v, k, k_len)) == -1) goto err;
    if(_dbf.flush_blocks() == -1) goto err;
    
//    if(val != NULL) _free_func((void**)&val);
    
    return re;
    err:
    char e[BUFSIZ];
    sprintf(e, "get value from index error: datafile %s, key=%s\n", _file_name.c_str(), k);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}
int kv_index::set(const char* k, const key_len_t k_len, const char* v, const value_len_t v_len)
{
    if(this->_set(k, k_len, v, v_len) == -1) goto err;
    if(_dbf.flush_blocks() == -1) goto err;
    _cache_insert(k, k_len, v, v_len);
    
    return 0;
    err:
    char e[BUFSIZ];
    sprintf(e, "set key-value in index error: datafile %s, key=%s value=%s\n", _file_name.c_str(), k, v);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}
int kv_index::del(char* k, key_len_t k_len)
{
    if(this->_del(k, k_len) == -1) goto err;
    if(_dbf.flush_blocks() == -1) goto err;
    return 0;
    err:
    char e[BUFSIZ];
    sprintf(e, "delete key-value from index error: datafile %s, key=%s\n", _file_name.c_str(), k);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}
    

/******************************************************************************************
*              init function
******************************************************************************************/
int kv_index::_open(bool create)
{
    int re = 0;
    if((re = _dbf.exists(_file_name)) == -1) return -1;
    if(re == 1) 
    {
        if(_dbf.open_file(_file_name) == -1) return -1;
    }
    else if(create)
    {
        if(_dbf.create_file(_dbname, _file_id, _file_name) == -1) return -1;
    }
    else return -1;
        
    if(_dbf.used_block_count() == 2 && _dbf.block_free_bytes(2) == KV_INDEX_NODE_LEN) if(_create() == -1) return -1;
    return 0;
}

int kv_index::_create()
{
    char index_head[KV_INDEX_NODE_LEN];
    node_head* h = NULL;
    memset(index_head, 0, KV_INDEX_NODE_LEN);
    h = (node_head*)index_head;
    h->block_id = 2;
    h->parent_block_id = 0;
    h->record_count = 0;
    h->right_child_block_id = 0;
    
    block_id_t block_id = 0;
    row_idx_t row_idx = 0;
    if(_dbf.write_row(&block_id, &row_idx, index_head, KV_INDEX_NODE_LEN) == -1) return -1;
    if(block_id != 2 || row_idx != 1) return -1;
    return 0;
}



/******************************************************************************************
*              record function
******************************************************************************************/
// return key length
int kv_index::_get_key_from_record(char** key, char* record)
{
    *key = record + sizeof(k_info);
    return ((k_info*)record)->key_len;
}

int kv_index::_cmp_key(const char* k1, const short len1, const char* k2, const int len2) const
{
    int len = len1 < len2 ? len1 : len2;
    int ret = memcmp(k1, k2, len);
    if(ret==0)
    {
        if(len1 == len2) return 0;
        else return (len1 > len2 ? 1 : -1);
    }
    else return ret;
}



/******************************************************************************************
*              node function
******************************************************************************************/
int kv_index::_get_node(char** node, const block_id_t block_id)
{
    return _dbf.read_row(node, block_id, 1);
}

int kv_index::_node_free_space(char* node)
{
    node_head* head = (node_head*)node;
    k_info* curr_k_info = NULL;
    char* curr_key = NULL;
    short curr_key_len = 0;
    
    int ret = KV_INDEX_NODE_LEN - sizeof(node_head);
    
    for(int i=0; i<head->record_count; i++)
    {
        if(curr_k_info == NULL)
        {
            curr_k_info = (k_info*)(node + sizeof(node_head));
            curr_key_len = _get_key_from_record(&curr_key, (char*)curr_k_info);
        }
        else
        {
            curr_k_info = (k_info*)(curr_key + curr_key_len);
            curr_key_len = _get_key_from_record(&curr_key, (char*)curr_k_info);
        }
        ret -= sizeof(k_info);
        ret -= curr_key_len;
    }
    ret -= sizeof(k_info);
    return ret;
}


int kv_index::_find_pos(char** pos, const char* node, const char* key, const short key_len)
{
    *pos = NULL;
    node_head* head = (node_head*)node;
    k_info* curr_k_info  = NULL;
    k_info* match_k_info = NULL;
    char* curr_key  = NULL;
    char* match_key = NULL;
    short curr_key_len  = 0;
    short match_key_len = 0;
    int space = KV_INDEX_NODE_LEN - sizeof(node_head);
    
    int cmp = 0;
    for(int i=0; i<head->record_count; i++)
    {
        if(curr_k_info == NULL)
        {
            curr_k_info = (k_info*)(node + sizeof(node_head));
            curr_key_len = _get_key_from_record(&curr_key, (char*)curr_k_info);
        }
        else
        {
            curr_k_info = (k_info*)(curr_key + curr_key_len);
            curr_key_len = _get_key_from_record(&curr_key, (char*)curr_k_info);
        }
        
        cmp = _cmp_key(curr_key, curr_key_len, key, key_len);
        if(cmp == 0)
        {
            *pos = (char*)curr_k_info;
            return 0;
        }
        if(cmp < 0) continue;
        
        if(match_k_info == NULL || _cmp_key(match_key, match_key_len, curr_key, curr_key_len) > 0)
        {
            match_k_info = curr_k_info;
            match_key_len = curr_key_len;
            match_key = curr_key;
        }
        space -= sizeof(curr_k_info);
        space -= curr_key_len;
    }
    
    if(match_k_info != NULL) *pos = (char*)match_k_info;
    else if(space >= (sizeof(k_info) + key_len))
    {
        if(curr_k_info == NULL) *pos = (char*)(node + sizeof(node_head));
        else *pos = curr_key + curr_key_len;
        k_info* info = (k_info*)(*pos);
        info->block_id = 0;
        info->key_len = 0;
        info->left_child_block_id = 0;
        info->row_idx = 0;
    }
    else *pos = NULL;
    return 0;
}


int kv_index::_add_node(int* block_id, const int parent_block_id)
{
    int node_len = KV_INDEX_NODE_LEN;
    char* node = new char[node_len];
    if(node == NULL)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    
    node_head* h = (node_head*)node;
    h->block_id = 0;
    h->parent_block_id = parent_block_id;
    h->right_child_block_id = 0;
    h->record_count = 0;
    
    if(_dbf.write_row_new_block(block_id, node, node_len)) return -1;
    
    h->block_id = *block_id;
    if(_save_node(node)) return -1;
    
    if(node != NULL) delete[] node;
    return 0;
}


int kv_index::_get_root(char** root)
{
    return _dbf.read_row(root, 2, 1);
}


int kv_index::_save_node(const char* node)
{
    return _dbf.modify_row(((node_head*)node)->block_id, 1, node, KV_INDEX_NODE_LEN);
}
    


/******************************************************************************************
 *              k-v function
 ******************************************************************************************/
// return value len
int kv_index::_get(char** val, const char*key, const key_len_t key_len)
{
    *val = NULL;
    block_id_t block_id = 0;
    row_idx_t row_idx = 0;
    char* root = NULL;
    if(_get_root(&root) == -1) return -1;
    if(_find_key(&block_id, &row_idx, root, key, key_len) == -1) return -1;
    if(root != NULL) delete[] root;
    if(block_id == 0) return 0;
    return _dbf.read_row(val, block_id, row_idx);
}

int kv_index::_del(const char* key, const key_len_t key_len)
{
    char* root = NULL;
    if(_get_root(&root) == -1) return -1;
    if(_del_key(root, key, key_len) == -1) return -1;
    if(root != NULL) delete[] root;
    return 0;
}

int kv_index::_del_key(const char* node, const char* key, const key_len_t key_len)
{
    char* pos = NULL;
    k_info* info = NULL;
    char* find_key = NULL;
    
    if(_find_pos(&pos, node, key, key_len) == -1) return -1;
    info = (k_info*)pos;
    if(pos == NULL || info->block_id == 0) return 0;
    find_key = pos + sizeof(k_info);
    if(_cmp_key(find_key, info->key_len, key, key_len) == 0)
    {
        if(_dbf.delete_row(info->block_id, info->row_idx) == -1) return -1;
        info->block_id = 0;
        info->row_idx = 0;
        _save_node(node);
    }
    return 0;
}


int kv_index::_find_key(block_id_t* block_id, row_idx_t* row_idx, const char* node, const char* key, const key_len_t key_len)
{
    char* pos = NULL;
    k_info* info = NULL;
    char* find_key = NULL;
    char* child_node = NULL;
    node_head* head = (node_head*)node;
    
    *block_id = 0;
    *row_idx = 0;
    if(_find_pos(&pos, node, key, key_len) == -1) return -1;
    
    
    if(pos != NULL)
    {
        info = (k_info*)pos;
        find_key = pos + sizeof(k_info);
        
        if(_cmp_key(find_key, info->key_len, key, key_len) == 0)
        {
            *block_id = info->block_id;
            *row_idx = info->row_idx;
        }
        else if(info->left_child_block_id != 0)
        {
            if(_get_node(&child_node, info->left_child_block_id) == -1) return -1;
            if(_find_key(block_id, row_idx, child_node, key, key_len) == -1) return -1;
        }
    }
    else
    {
        if(head->right_child_block_id != 0)
        {
            if(_get_node(&child_node, head->right_child_block_id) == -1) return -1;
            if(_find_key(block_id, row_idx, child_node, key, key_len) == -1) return -1;
        }
    }
    
    if(child_node != NULL) delete[] child_node;
    return 0;
}

int kv_index::_set(const char* key, const key_len_t key_len, const char* val, const int val_len)
{
    char* root = NULL;
    if(_get_root(&root) == -1) return -1;
    if(_set_key(root, key, key_len, val, val_len) == -1) return -1;
    if(root != NULL) delete[] root;
    return 0;
}

int kv_index::_set_key(const char* node, const char* key, const short key_len, const char* value, const int value_len)
{
    node_head* h = (node_head*)node;
    k_info* info = NULL;
    char* pos = NULL;
    
    if(_find_pos(&pos, node, key, key_len) == -1) return -1;
    if(pos == NULL)
    {
        block_id_t child_node_id = 0;
        char* child_node = NULL;
        if(h->right_child_block_id == 0)
        {
            // allocate new node
            if(_add_node(&child_node_id, h->block_id) == -1) return -1;
            h->right_child_block_id = child_node_id;
            if(_save_node(node) == -1) return -1;
        }
        else child_node_id = h->right_child_block_id;
            
        if(_get_node(&child_node, child_node_id) == -1) return -1;
        if(_set_key(child_node, key, key_len, value, value_len) == -1) return -1;
        if(child_node != NULL) delete[] child_node;
    }
    else
    {
        info = (k_info*)pos;
        if(info->block_id == 0)
        {
            // is new record in node
            block_id_t val_block_id = 0;
            row_idx_t  val_row_idx  = 0;
            if(_dbf.write_row(&val_block_id, &val_row_idx, value, value_len)) return -1;
            
            info->block_id = val_block_id;
            info->row_idx  = val_row_idx ;
            info->key_len  = key_len     ;
            info->left_child_block_id = 0;
            
            memcpy(pos+sizeof(k_info), key, key_len);
            h->record_count += 1;
            if(_save_node(node) == -1) return -1;
        }
        else if(_cmp_key(pos + sizeof(k_info), info->key_len, key, key_len) == 0) 
            return _dbf.modify_row(info->block_id, info->row_idx, value, value_len);
        else
        {
            block_id_t child_block_id = 0;
            char* child_node = NULL;
            if(info->left_child_block_id == 0)
            {
                // allocate new node
                if(_add_node(&child_block_id, h->block_id) == -1) return -1;
                info->left_child_block_id = child_block_id;
                if(_save_node(node) == -1) return -1;
            }
            else child_block_id = info->left_child_block_id;
            
            // goto child node
            if(_get_node(&child_node, child_block_id) == -1) return -1;
            if(_set_key(child_node, key, key_len, value, value_len) == -1) return -1;
            if(child_node != NULL) delete[] child_node;
        }
    }
    
    return 0;
}







/******************************************************************************************
*              cache function
******************************************************************************************/
int kv_index::_cache_get(char** val, const char* key, const key_len_t key_len)
{
    int v_len = 0;
    if(_lru.get(val, &v_len, key, key_len) == -1) return -1;
    return v_len;
}


int kv_index::_cache_insert(const char* key, const key_len_t key_len, const char* val, const value_len_t val_len)
{ return _lru.set(key, key_len, val, val_len); }








}
