/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   llist.h
 * Author: liusheng
 *
 * Created on 2016年6月28日, 下午5:03
 */

#ifndef LLIST_H
#define LLIST_H

#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

namespace pingkv
{
class llist
{
public:

    struct llist_node
    {
        void* data;
        llist_node* prev;
        llist_node* next;
    };
    
private:
    int _size;
    llist_node* _first;
    llist_node* _end;
    llist_node* _make_node(void* data, llist_node* prev, llist_node* next);
public:
    llist();
    ~llist();
    int init();
    
    int size();
    llist_node* first();
    int push_back(void* data);
    void del(llist_node* node);
    
};









}
#endif /* LLIST_H */

