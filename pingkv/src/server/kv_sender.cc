/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_sender.cc
 * Author: liusheng
 * 
 * Created on 2016年9月5日, 下午4:41
 */

#include "kv_sender.h"

namespace pingkv
{
kv_sender::kv_sender(kv_work_queue* res_queue): _res_queue(res_queue) {_module_name = "RESULT_SENDER";}
kv_sender::~kv_sender() {}

void kv_sender::thread_func()
{
    int fd = 0;
    kv_work* w = NULL;
    while(1)
    {
        for(std::map<int, kv_work*>::iterator it=_res_bufs.begin(); it!=_res_bufs.end(); it++)
        {
            fd = it->first;
            w = it->second;
            if(_do_send(fd, w) == -1)
            {
                close(fd);
                delete w;
                _res_bufs.erase(fd);
                break;
            }
        }
        
        _res_queue->lock_queue();
        if(_res_queue->size() == 0)
        {
            if(_res_bufs.size() == 0) _res_queue->wait_work();
            else
            {
                _res_queue->unlock_queue();
                continue;
            }
        }
        if((fd = _res_queue->get_work(&w)) == -1)
        {
            kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "get result from queue error\n");
            abort();
        }
        _res_bufs[fd] = w;
        _res_queue->unlock_queue();
    }
    return;
}


int kv_sender::_do_send(int fd, kv_work* w)
{
    int len = 0;
    if(w->head_offset < sizeof(w->head))
    {
        len = send(fd, (&w->head)+w->head_offset, sizeof(kv_work_head)-w->head_offset, 0);
        if(len == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            goto err;
        }
        w->head_offset += len;
    }
    else if(w->field1_offset < w->head.field1_len)
    {
        len = send(fd, w->field1+w->field1_offset, w->head.field1_len-w->field1_offset, 0);
        if(len == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            goto err;
        }
        w->field1_offset += len;
    }
    else if (w->field2_offset < w->head.field2_len)
    {
        len = send(fd, w->field2+w->field2_offset, w->head.field2_len-w->field2_offset, 0);
        if(len == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            goto err;
        }
        w->field2_offset += len;
    }

    if(w->head_offset == sizeof(w->head) && w->field1_offset == w->head.field1_len && w->field2_offset == w->head.field2_len)
    {
        _res_bufs.erase(fd);
        delete w;
    }
    
    return 0;
    err:
    return -1;
}



}

