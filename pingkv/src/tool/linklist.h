/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   linklist.h
 * Author: liusheng
 *
 * Created on 2016年5月4日, 下午1:41
 */

#ifndef LINKLIST_H
#define LINKLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include "container_default_funcs.h"

using namespace std;

template<typename T>
class linklist
{
public:
    typedef boost::function<int(const T&, const T&)> comp_func_t;
    typedef boost::function<void*(const size_t)> alloc_func_t;
    typedef boost::function<void*(const void*)> free_func_t;
    
    struct node
    {
        T data;
        node* prev;
        node* next;
    };
    
    struct Iterator
    {
        node* _n;
        explicit Iterator(node* n):_n(n){}
        inline T& operator*() const {return _n->data;}
        inline T* operator->() const {&(_n->data);}
        inline Iterator& operator++() {_n = _n->next; return *this;};
        inline Iterator& operator--() {_n = _n->prev; return *this;};
        inline Iterator& operator++(int) {_n = _n->next; return *this;};
        inline Iterator& operator--(int) {_n = _n->prev; return *this;};
        
        inline bool operator==(const Iterator& i) const {return _n == i._n;};
        inline bool operator!=(const Iterator& i) const {return _n != i._n;};
    };
    Iterator begin() {return Iterator(_begin);}
    Iterator end() {return Iterator(NULL);}
    node* first_node(){return _begin;};
    node* last_node() {return _end;};

private:        
    ulong _size;
    linklist<T>::node* _begin;
    linklist<T>::node* _end;
    alloc_func_t _alloc_func;
    free_func_t  _free_func;
    comp_func_t  _comp_func;
    void _insert_front(linklist<T>::node* n, linklist<T>::node* new_node);
    void _insert_after(linklist<T>::node* n, linklist<T>::node* new_node);
    void _del_node(const node* n);
    /*
     *  return value
     *  data ptr ----------> seccess
     *  NULL     ----------> data not found
     * */
    T* linklist_delete(T* data);
    
    linklist( const linklist& );
    const linklist& operator=( const linklist& ); 
    
    
public:
    linklist();
    linklist(const alloc_func_t alloc_func, const free_func_t free_func, const comp_func_t comp_func);
    ~linklist();
    void init(const alloc_func_t alloc_func, const free_func_t free_func, const comp_func_t comp_func);
    ulong size() const;
    
    /*
     free all nodes
    */
    void destroy_nodes();
    
    node* contains(const T& data) const
    {
        linklist<T>::node* n = _begin;
        while(n)
        {
            if(_comp_func(n->data, data) == 0) break;
            n=n->next;
        }
        return n;
    }
    
    /*
     return value
     * true  -----> delete success
     * false -----> data not found
     */
    bool del(const T& data);
    void del(const node* n);
    void append(const T& data);
    

} ;


template<typename T>
linklist<T>::linklist()
{
    init(
    boost::bind(default_alloc, _1),
    boost::bind(default_free, _1),
    boost::bind(default_compare<T>, _1, _2)
    );
}

template<typename T>
linklist<T>::linklist(const alloc_func_t alloc_func, const free_func_t free_func, const comp_func_t comp_func)
{
    init(alloc_func, free_func, comp_func);
}

template<typename T>
linklist<T>::~linklist()
{
    destroy_nodes();
}

template<typename T>
void linklist<T>::_insert_front(linklist<T>::node* n, linklist<T>::node* new_node)
{
    if(n == NULL) return;
    linklist<T>::node* pre = n->prev;
    new_node->prev = pre;
    new_node->next = n;
    n->prev = new_node;
    if(pre != NULL) pre->next = new_node;
}

template<typename T>
void linklist<T>::_insert_after(linklist<T>::node* n, linklist<T>::node* new_node)
{
    if(n == NULL) return;
    linklist<T>::node* nxt = n->next;
    new_node->prev = n;
    new_node->next = nxt;
    n->next = new_node;
    if(nxt != NULL) nxt->prev = new_node;
}

template<typename T>
void linklist<T>::_del_node(const linklist<T>::node* n)
{
    assert(n);
    linklist<T>::node* pre = n->prev;
    linklist<T>::node* nxt = n->next;
    if(pre != NULL) pre->next = nxt;
    if(nxt != NULL) nxt->prev = pre;
}



template<typename T>
void linklist<T>::init(const alloc_func_t alloc_func, const free_func_t free_func, const comp_func_t comp_func)
{
    _size  = 0;
    _begin = NULL;
    _end   = NULL;
    _alloc_func = alloc_func;
    _free_func  = free_func;
    _comp_func  = comp_func;
}

template<typename T>
void linklist<T>::destroy_nodes()
{
    node* n1=_begin, *n2=NULL;
    while(_size > 0)
    {
        n2 = n1->next;
        _free_func(n1);
        n1 = n2;
        --_size ;
    }
    _begin = NULL;
    _end = NULL;
}

template<typename T>
ulong linklist<T>::size() const
{
    return _size;
}



template<typename T>
void linklist<T>::append(const T& data)
{   
    linklist<T>::node* n = (linklist<T>::node*)_alloc_func(sizeof(linklist<T>::node));
    n->prev = NULL;
    n->next = NULL;
    n->data = data;

    if(_size == 0)
    {
        _begin = n;
        _end = n;
    }
    else 
    {
        _insert_after(_end, n);
        _end = n;
    }
    _size += 1;
}

template<typename T>
bool linklist<T>::del(const T& data)
{
    int i=0;
    
    linklist<T>::node* n = contains(data);
    if(n)
    {
        del(n);
        return true;
    }
    return false;
}

template<typename T>
void linklist<T>::del(const node* n)
{
    if(n == _begin) _begin = n->next;
    if(n == _end  ) _end   = n->prev;
    _del_node(n);
    _size --;
    _free_func(n);
}


/*
 TEST
int my_comp(const void* d1, const void* d2)
{
    cout << "my_compare" << endl;
    pair<int,string>* p1 = *(pair<int,string>**)d1;
    pair<int,string>* p2 = *(pair<int,string>**)d2;
    
    if ((p1==NULL && p2==NULL) || (p1 && p2 && p1->first == p2->first)) return 0;
    else if (p2 == NULL || (p1)->first > (p2)->first) return 1;
    else return -1;
}

void* my_alloc(size_t size)
{
    cout << "my_alloc" << endl;
    void* ptr = malloc(size);
    assert(ptr);
    return ptr;
}


void my_free(void* ptr)
{
    cout << "my_free" << endl;
    if(ptr != NULL)
    {
        free(ptr);
        ptr = NULL;
    }
}

int main(int argc, char** argv) {
    linklist<pair<int,string>* > ls(my_alloc, my_free, my_comp);
    
    pair<int,string> p1(1, "aa");
    pair<int,string> p2(2, "bb");
    pair<int,string> p3(3, "cc");
    ls.append(&p1);
    ls.append(&p2);
    ls.append(&p3);
    
    for(linklist<pair<int,string>* >::Iterator i = ls.begin(); i!=ls.end(); i++)
    {
        cout << (*i)->first << " " << (*i)->second << endl;
    }
    
    pair<int,string> p4(1, "xx");
    pair<int,string> p5(2, "yy");
    pair<int,string> p6(3, "zz");
    pair<int,string> p7(4, "zz");
    cout << ls.contains(&p4) << endl;
    cout << ls.contains(&p5) << endl;
    cout << ls.contains(&p6) << endl;
    cout << ls.contains(&p7) << endl;
    return 0;
}
 */

#endif /* LINKLIST_H */

