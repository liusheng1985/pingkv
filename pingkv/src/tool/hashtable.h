#ifndef LSHASHTABLE_H
#define LSHASHTABLE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include "hash.h"
#include "linklist.h"
#include "my_mutex.h"
#include "container_default_funcs.h"
using namespace std;




template <typename K, typename V>
class hash_table
{
public: 
    /******************************************************************************************
     *              struct, typedef
     ******************************************************************************************/
    struct element
    {
        K k;
        V v;
        element(K key):k(key){}
        element(K key, V val):k(key), v(val){}
    };
    typedef hash_table<K,V>::element* p_elemet_t;
    typedef linklist<p_elemet_t> data_list_t;
    
    struct bucket
    {
        data_list_t data_list;
        my_mutex mutex;
    };
    
    
    typedef boost::function<long(const K&)> hash_func_t;
    typedef boost::function<int(const K&, const K&)> comp_func_t;
    typedef boost::function<void*(const size_t)> alloc_func_t;
    typedef boost::function<void*(const void*)> free_func_t;
    
private:
    /******************************************************************************************
     *              callback function
     ******************************************************************************************/
    hash_func_t  _hash_func;
    comp_func_t  _comp_func;
    alloc_func_t _alloc_func;
    free_func_t  _free_func;
    
    
    my_mutex _table_mutex;
    long _size;
    long _bucket_count;
    V _default_value;
    hash_table<K, V>::bucket* _buckets;
    
    /******************************************************************************************
     *              bucket function
     ******************************************************************************************/
    void _init_buckets(bucket** buckets_ptr, long count);
    void _free_buckets(bucket* bkt, long count);
    // make bucket_count to x^n from 128.  128, 256, 512 ... 1024 ...
    int _make_bucket_count(int bucket_count);
    
    
    /******************************************************************************************
     *              element function
     ******************************************************************************************/
    void _new_elem(p_elemet_t* e, const K& k, const V& v);
    /**
     * return old element*
     */
    int _add_elem(bucket* bkt, const K& k, const V& v);
    
    
public:
    /******************************************************************************************
     *              constructor, init, destroy
     ******************************************************************************************/
    hash_table(int bucket_count, const V& default_value, alloc_func_t alloc_func, free_func_t free_func, hash_func_t hash_func, comp_func_t comp_func);
    void init(int bucket_count, const V& default_value, alloc_func_t alloc_func, free_func_t free_func, hash_func_t hash_func, comp_func_t comp_func);
    
    ~hash_table();
    void destroy();
    
    /******************************************************************************************
     *             iterator
     ******************************************************************************************/
    struct Iterator
    {
    private:
        typename data_list_t::node* _n;
        hash_table<K, V>* _table;
        hash_table<K, V>::bucket* _bs;
        long _idx;
    public:
        explicit Iterator(typename data_list_t::node* n, hash_table<K, V>::bucket* bs, long idx, hash_table<K, V>* tab):_n(n), _idx(idx), _bs(bs), _table(tab) {}
        inline p_elemet_t operator*() const {return _n ? _n->data : NULL;}
        inline p_elemet_t* operator->() const {return _n ? &(_n->data) : NULL;}
        
        
        inline Iterator& operator++() 
        {
            _n = _n->next;
            if(_n == NULL)
            {
                while(_idx < _table->bucket_count()-1)
                {
                    ++_idx;
                    if((_bs + _idx)->data_list.size() > 0)
                    {
                        _n = (_bs + _idx)->data_list.first_node();
                        break;
                    }
                }
            }
            return *this;
        }
        
        inline Iterator& operator--()
        {
            _n = _n->prev;
            if(_n == NULL)
            {
                while(_idx>0)
                {
                    --_idx;
                    if((_bs + _idx)->data_list.size() > 0)
                    {
                        _n = (_bs + _idx)->data_list.first_node();
                        break;
                    }
                }
            }
            return *this;
        }
        
        inline Iterator& operator++(int)
        {
            _n = _n->next;
            if(_n == NULL)
            {
                while(_idx < _table->bucket_count()-1)
                {
                    ++_idx;
                    if((_bs + _idx)->data_list.size() > 0)
                    {
                        _n = (_bs + _idx)->data_list.first_node();
                        break;
                    }
                }
            }
            return *this;
        }
        
        inline Iterator& operator--(int)
        {
            _n = _n->prev;
            if(_n == NULL)
            {
                while(_idx>0)
                {
                    --_idx;
                    if((_bs + _idx)->data_list.size() > 0)
                    {
                        _n = (_bs + _idx)->data_list.first_node();
                        break;
                    }
                }
            }
            return *this;
        }
        
        inline bool operator==(const Iterator& i) const 
        {
            if(_n) return _table->comp_elem(_n->data, *i) == 0;
            return *i == NULL;
        }
        inline bool operator!=(const Iterator& i) const 
        {
            if(_n) return _table->comp_elem(_n->data, *i) != 0;
            return *i != NULL;
        }
    };
    
    Iterator begin() 
    {
        for(long i=0; i<_bucket_count; i++) if((_buckets+i)->data_list.size() > 0) return Iterator((_buckets+i)->data_list.first_node(), _buckets, i, this);
        return Iterator(NULL, _buckets, _bucket_count-1, this);
    }
    Iterator end() {return Iterator(NULL, _buckets, _bucket_count-1, this);}
    
    /******************************************************************************************
     *              user interface
     ******************************************************************************************/
    long size();
    long bucket_count();
    V find(const K& k);
    bool add_or_replace(const K& k, const V& v);
    bool del(const K& k);
    int comp_elem(const p_elemet_t& data1, const p_elemet_t& data2);
};



/******************************************************************************************
*              constructor, init, destroy
******************************************************************************************/
template <typename K, typename V> hash_table<K, V>::~hash_table() {destroy();}
template <typename K, typename V> hash_table<K, V>::hash_table(int bucket_count, const V& default_value, alloc_func_t alloc_func, free_func_t free_func, hash_func_t hash_func, comp_func_t comp_func) 
{init(bucket_count, default_value, alloc_func, free_func, hash_func, comp_func);}

template <typename K, typename V> void hash_table<K, V>:: destroy()
{
    my_mutex_locker table_lock(_table_mutex);
    _free_buckets(_buckets, _bucket_count);
}


template <typename K, typename V>
void 
hash_table<K, V>::
init(int bucket_count, const V& default_value, alloc_func_t alloc_func, free_func_t free_func, hash_func_t hash_func, comp_func_t comp_func)
{
    _alloc_func = alloc_func;
    _free_func  = free_func;
    _comp_func  = comp_func;
    _hash_func  = hash_func;
    _bucket_count = _make_bucket_count(bucket_count);
    _size = 0;
    _default_value = default_value;
    _init_buckets(&_buckets, _bucket_count);
}



/******************************************************************************************
 *              user interface
 ******************************************************************************************/
template <typename K, typename V> long hash_table<K, V>::size() {return _size;}
template <typename K, typename V> long hash_table<K, V>::bucket_count() {return _bucket_count;}

template <typename K, typename V>
V
hash_table<K, V>::
find(const K& k)
{
    hash_table<K, V>::bucket* bkt = NULL;
    {
        my_mutex_locker table_lock(_table_mutex);
        bkt = _buckets + (_hash_func(k) & (_bucket_count - 1));
        bkt->mutex.lock();
    }
    element ele(k);
    typename data_list_t::node* n = bkt->data_list.contains(&ele);
    bkt->mutex.unlock();
    if(n) return n->data->v;
    else return _default_value;
    my_mutex_locker table_lock(_table_mutex);
}

template <typename K, typename V>
bool
hash_table<K, V>::
add_or_replace(const K& k, const V& v)
{
    my_mutex_locker table_lock(_table_mutex);
    hash_table<K, V>::bucket* bkt = _buckets + (_hash_func(k) & (_bucket_count - 1));
    {
        my_mutex_locker bucket_lock(bkt->mutex);
        int r = _add_elem(bkt, k, v);
        _size += r;
        return r;
    }
}

template <typename K, typename V>
bool
hash_table<K, V>::
del(const K& k)
{
    my_mutex_locker table_lock(_table_mutex);
    hash_table<K, V>::bucket* bkt = _buckets + (_hash_func(k) & (_bucket_count - 1));
    {
        my_mutex_locker bucket_lock(bkt->mutex);
        element ele(k);
        typename data_list_t::node* n = bkt->data_list.contains(&ele);
        if(!n) return false;
        
        _free_func(n->data);
        bkt->data_list.del(n);
        --_size;
        return true;
    }
}


/******************************************************************************************
 *              bucket function
 ******************************************************************************************/
template <typename K, typename V>
int
hash_table<K, V>::
_make_bucket_count(int bucket_count)
{
    int i=128;
    while(i<bucket_count) i*=2;
    return i;
}

template <typename K, typename V>
void
hash_table<K, V>::
_init_buckets(bucket** buckets_ptr, long count)
{
    *buckets_ptr = (bucket*)_alloc_func(sizeof(bucket) * count);
    assert(*buckets_ptr);
    bucket* bkt = NULL;
    for(int i=0; i<count; i++)
    {
        bkt = (*buckets_ptr) + i;
        bkt->data_list.init(_alloc_func, _free_func, boost::bind(&hash_table<K,V>::comp_elem, this, _1,_2));
    }
}

template <typename K, typename V>
void
hash_table<K, V>::
_free_buckets(bucket* bkt, long count)
{
    bucket* b;
    for(int i=0; i<count; i++)
    {
        b = bkt + i;
        b->data_list.destroy_nodes();
    }
    _free_func(bkt);
}


/******************************************************************************************
 *              element function
 ******************************************************************************************/
template <typename K, typename V>
void
hash_table<K, V>::
_new_elem(p_elemet_t* e, const K& k, const V& v)
{
    *e = (p_elemet_t)_alloc_func(sizeof(element));
    assert(*e);
    (*e)->k = k;
    (*e)->v = v;
}

template <typename K, typename V>
int 
hash_table<K, V>::
comp_elem(const p_elemet_t& data1, const p_elemet_t& data2)
{
    if(data1  ==  data2 || (data1==NULL && data2==NULL)) return 0;
    if(data1  && !data2) return 1;
    if(!data1 &&  data2) return -1;
    return _comp_func(data1->k, data2->k);
}

template <typename K, typename V>
int
hash_table<K, V>::
_add_elem(bucket* bkt, const K& k, const V& v)
{
    assert(bkt);
    p_elemet_t ele = NULL;
    _new_elem(&ele, k, v);
    typename data_list_t::node* n = bkt->data_list.contains(ele);
    if(n)
    {
        ele = (p_elemet_t)_free_func(ele);
        n->data->k = k;
        n->data->v = v;
        return 0;
    }
    else 
    {
        bkt->data_list.append(ele);
        return 1;
    }
}

#endif /* LSHASHTABLE_H */



