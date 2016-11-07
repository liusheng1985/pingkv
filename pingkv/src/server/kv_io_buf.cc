/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_io_buf.cc
 * Author: liusheng
 * 
 * Created on 2016年8月11日, 上午11:39
 */

#include "kv_io_buf.h"

namespace pingkv
{
    
kv_io_buf::kv_io_buf():kv_base(), _block_size(BUFSIZ) {}
kv_io_buf::kv_io_buf(int block_size): kv_base(), _block_size(block_size) {}
kv_io_buf::~kv_io_buf() {while(!_blocks.empty()) _del_queue_front();}

char* kv_io_buf::_push_new_block()
{
    block* b = NULL;
    
    if((b = new block) == NULL) return NULL;
    if((b->data = new char[_block_size]) == NULL) return NULL; 
    b->r_offset = 0;
    b->w_offset = 0;
    _blocks.push(b);
    return b->data;
}

void kv_io_buf::_del_queue_front()
{
    if(_blocks.size() == 0) return;
    block* b = _blocks.front();
    delete[] b->data;
    delete b;
    _blocks.pop();
}


int kv_io_buf::size()
{
    if(_blocks.size() == 0) return 0;
    block* r = _blocks.front();
    block* w = _blocks.back();
    if(r == w) return w->w_offset - r->r_offset;
    return (_block_size - r->r_offset) + (w->w_offset) + (_blocks.size() - 2)*_block_size;
}


int kv_io_buf::block_size() {return _block_size;}


char* kv_io_buf::r_ptr()
{
    if(_blocks.size() == 0) return _push_new_block();
    block* b = _blocks.front();
    return b->data + b->r_offset;
}

int kv_io_buf::r_len()
{
    if(_blocks.size() == 0) _push_new_block();
    block* b = _blocks.front();
    if(b->r_offset == _block_size)
    {
        _del_queue_front();
        return r_len();
    }
    return b->w_offset - b->r_offset;
}


char* kv_io_buf::w_ptr()
{
    if(_blocks.size() == 0) return _push_new_block();
    block* b = _blocks.back();
    if(b->w_offset == _block_size) return _push_new_block();
    return b->data + b->w_offset;
}

int kv_io_buf::w_len()
{
    if(_blocks.size() == 0) _push_new_block();
    block* b = _blocks.back();
    if(b->w_offset == _block_size) _push_new_block();
    b = _blocks.back();
    return _block_size - b->w_offset;
}


int kv_io_buf::add_r_offset(int add)
{
    block* b = _blocks.front();
    assert(add <= (b->w_offset - b->r_offset));
    b->r_offset += add;
    if(b->r_offset == _block_size) _del_queue_front();
    return 0;
}

int kv_io_buf::add_w_offset(int add)
{
    block* b = _blocks.back();
    assert(add <= (_block_size - b->w_offset));
    b->w_offset += add;
    return 0;
}



}