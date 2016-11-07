/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   llist.cc
 * Author: liusheng
 * 
 * Created on 2016年6月28日, 下午5:03
 */

#include "llist.h"

namespace pingkv
{
llist::llist()
{
    _size = 0;
    _first = NULL;
    _end = NULL;
}

llist::~llist() {while(_first != NULL) del(_first);}

int llist::init()
{
    _size = 0;
    _first = NULL;
    _end = NULL;
}

int llist::push_back(void* data)
{
    llist_node* n = _make_node(data, _end, NULL);
    if(n == NULL) return -1;
    
    if(_first == NULL) _first = n;
    if(_end) _end->next = n;
    _end = n;
    _size += 1;
    return 0;
}

llist::llist_node* llist::_make_node(void* data, llist_node* prev, llist_node* next)
{
    llist_node* n = new llist_node;
    if(n == NULL) return NULL;
    n->data = data;
    n->prev = prev;
    n->next = next;
    return n;
}


void llist::del(llist_node* node)
{
    assert(node != NULL);
    if(node->prev) node->prev->next = node->next;
    if(node->next) node->next->prev = node->prev;
    if(_first == node) _first = node->next;
    if(_end   == node) _end   = node->prev;
    _size -= 1;
    delete node;
}

int llist::size() {return _size;}
llist::llist_node* llist::first() {return _first;}





}