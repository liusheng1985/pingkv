/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_server.h
 * Author: liusheng
 *
 * Created on 2016年7月12日, 下午2:01
 */

#ifndef KV_SERVER_H
#define KV_SERVER_H

#include "../tool/kv_base.h"
#include "../conf/kv_conf.h"
#include "../tool/kv_poller.h"
#include "../dbf/data_file_tool.h"
#include "../tool/tool.h"
#include "kv_work_queue.h"
#include "kv_io_buf.h"
#include "kv_worker.h"
#include "kv_sender.h"


namespace pingkv
{
class kv_server: public kv_base
{
public:

private:
    int _listen_sock;
    int _local_file_count;
    int _fileno_low;
    int _fileno_high;
    kv_conf_data _cd;
    std::vector<kv_work_queue*> _req_queue;
    kv_work_queue* _res_queue;
    
    
    // workers
    std::vector<kv_worker*> _workers;
    std::vector<boost::thread*> _threads; // one thread per worker
    
    // result sender
    kv_sender* _sender;
    boost::thread* _send_thread;
    
    // network io buf
    std::map<int, kv_work*> _work_bufs;
    
    kv_poller _pl;
    int _new_work_buf(int fd);
    int _no_block(int fd);
    int _do_listen();
    void _cb_accept(int fd, void* arg);
    void _cb_read_sock (int fd, void* arg);
    void _cb_write_sock(int fd, void* arg);
    
    
public:
    kv_server(const kv_conf_data& cd);
    virtual ~kv_server();
    
    int serv();
    
};
}
#endif /* KV_SERVER_H */




