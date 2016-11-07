/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_request_qfer.cc
 * Author: liusheng
 * 
 * Created on 2016年7月12日, 下午2:03
 */

#include "kv_work_queue.h"

namespace pingkv
{

kv_work_queue::kv_work_queue()
{
    assert(pthread_mutex_init(&_x, NULL) != -1);
    assert(pthread_cond_init(&_c, NULL) != -1);
    _module_name = "WORK_QUEUE";
}


kv_work_queue::~kv_work_queue()
{
    pthread_mutex_destroy(&_x);
    pthread_cond_destroy(&_c);
    kv_pack_buf_elem* elem = NULL;
    while(_q.size() > 0)
    {
        elem = _q.front();
        if(elem != NULL)
        {
            delete elem->data;
            delete elem;
        }
        _q.pop();
    }
}


bool kv_work_queue::is_empty() {return (_q.size() == 0);}
int  kv_work_queue::size() {return _q.size();}


int kv_work_queue::lock_queue   () {if(pthread_mutex_lock  (&_x)      != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}
int kv_work_queue::unlock_queue () {if(pthread_mutex_unlock(&_x)      != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}
int kv_work_queue::wait_work    () {if(pthread_cond_wait   (&_c, &_x) != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}
int kv_work_queue::signal_waiter() {if(pthread_cond_signal (&_c)      != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}

int kv_work_queue::put_work(int fd, kv_work* data)
{
    kv_pack_buf_elem* elem = new kv_pack_buf_elem;
    if(elem == NULL) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    elem->fd = fd;
    elem->data = data;
    _q.push(elem);
    return 0;
}

int kv_work_queue::get_work(kv_work** data)
{
    if(_q.size() == 0)
    {
        *data = NULL;
        return 0;
    }

    kv_pack_buf_elem* elem = _q.front();
    _q.pop();
    
    *data = elem->data;
    return elem->fd;
}

////////////////////////////////////////////////////////////////////////////////
kv_resault_map::kv_resault_map(): kv_base()
{
    assert(pthread_mutex_init(&_x, NULL) != -1);
    assert(pthread_cond_init(&_c, NULL) != -1);
}


kv_resault_map::~kv_resault_map()
{
    pthread_mutex_destroy(&_x);
    pthread_cond_destroy(&_c);
    for(std::map<int, kv_work_queue*>::iterator it=_res.begin(); it!=_res.end(); it++) delete it->second;
}


bool kv_resault_map::is_empty() {return (_res.size() == 0);}
int  kv_resault_map::size() {return _res.size();}


int kv_resault_map::lock_map      () {if(pthread_mutex_lock  (&_x)      != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}
int kv_resault_map::unlock_map    () {if(pthread_mutex_unlock(&_x)      != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}
int kv_resault_map::wait_resault  () {if(pthread_cond_wait   (&_c, &_x) != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}
int kv_resault_map::signal_waiter () {if(pthread_cond_signal (&_c)      != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);}

int kv_resault_map::put_resault(int fd, kv_work* data)
{
    if(_res[fd] == NULL) _res[fd] = new kv_work_queue();
    return _res[fd]->put_work(fd, data);
}

int kv_resault_map::get_resault(int fd, kv_work** data)
{
    if(_res.size() == 0)
    {
        *data = NULL;
        return 0;
    }

    kv_work_queue* q = _res[fd];
    if(q == NULL || q->size() == 0) *data = NULL;
    else 
    {
        q->get_work(data);
        if(q->size() == 0) _res.erase(fd);
        return fd;
    }
}

}
