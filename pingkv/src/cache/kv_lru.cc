/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_lru.cc
 * Author: liusheng
 * 
 * Created on 2016年6月28日, 下午1:24
 */

#include "kv_lru.h"
namespace pingkv
{


kv_lru::kv_lru() {_module_name = "CACHE";}
int kv_lru::init(int bucket_count, int max_size, int max_elem_size)
{
    _bucket_count = bucket_count;
    _size = 0;
    _max_size = max_size; 
    _max_elem_size = max_elem_size; 
    
    _lru.node_count = 0;
    _lru.cold = NULL;
    _lru.hot = NULL;
    
    
    if((_tab = new llist[_bucket_count]) == NULL) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    llist* ls = NULL;
    
    
    for(int i=0; i<_bucket_count; i++)
    {
        ls = _tab + i;
        ls->init();
    }
    return 0;
}

kv_lru::~kv_lru() {}

int kv_lru::size() const
{
    return _size;
}

int kv_lru::set(const char* k, int k_len, const char* v, int v_len)
{
    if(k_len + v_len > _max_elem_size) return 0;
    if(_size > _max_size) if(_ageout() == -1) goto err;
    if(_set(k, k_len, v, v_len) == -1) goto err;
    return 0;
    
    err:
    char e[BUFSIZ];
    sprintf(e, "insert data to cache error:\nkey=%s\nvalue=%s\n", k, v);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}

int kv_lru::get(char** v, int* v_len, const char* k, int k_len)
{
    if(_get(v, v_len, k, k_len) == -1) goto err;
    return 0;
    
    err:
    char e[BUFSIZ];
    sprintf(e, "get data from cache error: %s\n", k);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}

int kv_lru::_lru_hash(const char* data, int data_len, int mod)
{
    long h = hash_data(data, data_len);
    return (h>0 ? h%mod : (-1*h%mod));
}

int kv_lru::_init_lru_node(lru_node** node, const char* k, int k_len, const char* v, int v_len, lru_node* prev, lru_node* next)
{
    if((*node = new lru_node) == NULL ) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    (*node)->ref_count = 0;
    (*node)->prev = prev;
    (*node)->next = next;
    (*node)->k_len = k_len;
    (*node)->v_len = v_len;
    if(((*node)->k = new char[k_len]) == NULL) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    if(((*node)->v = new char[v_len]) == NULL) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    return 0;
}



int kv_lru::_lru_insert(lru_node* n)
{
    if(_lru.hot) _lru.hot->prev = n;
    n->next = _lru.hot;
    _lru.hot = n;
    if(_lru.cold == NULL) _lru.cold = n;
    _lru.node_count += 1;
    _size += n->k_len;
    _size += n->v_len;
    return 0;
}

int kv_lru::_lru_del(lru_node* n)
{
    lru_node* pre = n->prev;
    lru_node* nex = n->next;
    if(_lru.hot  == n) _lru.hot  = nex;
    if(_lru.cold == n) _lru.cold = pre;
    
    if(pre) pre->next = nex;
    if(nex) nex->prev = pre;
    
    n->prev = NULL;
    n->next = NULL;
    
    _lru.node_count -= 1;
    _size -= n->k_len;
    _size -= n->v_len;
    
    return 0;
}

int kv_lru::_set(const char* k, int k_len, const char* v, int v_len)
{
    llist* ls = _tab + _lru_hash(k, k_len, _bucket_count);
    lru_node* ln = NULL;
    if(_get(&ln, k, k_len) == -1) return -1;
    if(ln)
    {
        ln->v_len = v_len;
        delete[] ln->v;
        ln->v = new char[v_len];
        memcpy(ln->v, v, v_len);
    }
    else
    {
        if(_init_lru_node(&ln, k, k_len, v, v_len, NULL, NULL) == -1) return -1;
        if(_lru_insert(ln) == -1) return -1;
        if(ls->push_back(ln) == -1) return -1;
    }
    
    return 0;
}

int kv_lru::_get(char** v, int* v_len, const char* k, int k_len)
{
    *v = NULL;
    lru_node* node = NULL;
    if(_get(&node, k, k_len) == -1) return -1;
    if(node == NULL) return 0;
    *v = new char[node->v_len];
    memcpy(*v, node->v, node->v_len);
    *v_len = node->v_len;
    return 0;
}

int kv_lru::_get(lru_node** node, const char* k, int k_len)
{
    llist* ls = _tab + _lru_hash(k, k_len, _bucket_count);
    *node = NULL;
    lru_node* ln = NULL;
    if(ls->first() != NULL)
    for(llist::llist_node* n=ls->first(); n!=NULL; n=n->next)
    {
        ln = (lru_node*)n->data;
        if(k_len = ln->k_len && ::memcmp(k, ln->k, k_len) == 0)
        {
            if(_lru_del   (ln) == -1) return -1;
            if(_lru_insert(ln) == -1) return -1;
            *node = ln;
            return 0;
        }
    }
    return 0;
}


int kv_lru::_del(lru_node* n)
{
    if(_lru_del(n) == -1) return -1;
    
    llist* ls = _tab + _lru_hash(n->k, n->k_len, _bucket_count);
    if(ls->first())
    for(llist::llist_node* i=ls->first(); i!=NULL; i=i->next)
    {
        if(n == i->data)
        {
            delete[] n->k;
            delete[] n->v;
            delete n;
            ls->del(i);
            break;
        }
    }
    return 0;
}


int kv_lru::_ageout()
{
    while(_size > _max_size/2) if(_del(_lru.cold) == -1) return -1;
    return 0;
}

}





