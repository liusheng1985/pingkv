/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_request_buffer.h
 * Author: liusheng
 *
 * Created on 2016年7月12日, 下午2:03
 */

#ifndef KV_REQUEST_BUFFER_H
#define KV_REQUEST_BUFFER_H



#include "../tool/kv_base.h"
#include "kv_work.h"

namespace pingkv
{
    
class kv_work_queue: public kv_base
{
public:
    struct kv_pack_buf_elem
    {
        int fd;
        kv_work* data; 
    };
private:
    std::queue<kv_pack_buf_elem*> _q;
    pthread_mutex_t _x;
    pthread_cond_t  _c;
public:
    kv_work_queue();
    virtual ~kv_work_queue();
    
    bool is_empty();
    int size();
    
    // pthread_mutex_lock
    int lock_queue();
    // pthread_mutex_unlock
    int unlock_queue();
    // pthread_cond_wait
    int wait_work();
    // pthread_cond_signal
    int signal_waiter();
    
    int put_work(int fd, kv_work* data);
    // return fd
    int get_work(kv_work** data);
    
};

class kv_resault_map: public kv_base
{
private:
    std::map<int, kv_work_queue*> _res;
    pthread_mutex_t _x;
    pthread_cond_t  _c;
public:
    kv_resault_map();
    virtual ~kv_resault_map();
    
    bool is_empty();
    int size();
    
    // pthread_mutex_lock
    int lock_map();
    // pthread_mutex_unlock
    int unlock_map();
    // pthread_cond_wait
    int wait_resault();
    // pthread_cond_signal
    int signal_waiter();
    
    int put_resault(int fd, kv_work* data);
    // return sock
    int get_resault(int fd, kv_work** data);
    
};

}

#endif /* KV_REQUEST_BUFFER_H */

