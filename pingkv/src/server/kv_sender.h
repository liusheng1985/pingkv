/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_sender.h
 * Author: liusheng
 *
 * Created on 2016年9月5日, 下午4:41
 */

#ifndef KV_SENDER_H
#define KV_SENDER_H

#include "../tool/kv_base.h"
#include "kv_work_queue.h"
#include "kv_io_buf.h"
#include "kv_worker.h"

using std::cout;
using std::endl;
namespace pingkv
{

class kv_sender: public kv_base
{
private:
    kv_work_queue* _res_queue;
    std::map<int, kv_work*> _res_bufs;
    
    int _do_send(int fd, kv_work* w);
public:
    kv_sender(kv_work_queue* res_queue);
    virtual ~kv_sender();

    void thread_func();
};
}
#endif /* KV_SENDER_H */

