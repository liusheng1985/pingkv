/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_worker.cc
 * Author: liusheng
 * 
 * Created on 2016年7月12日, 下午2:02
 */

#include "kv_worker.h"
namespace pingkv
{
kv_worker::kv_worker(const string& db_name, int file_id, const string& data_file_name, bool create, kv_work_queue* request_buf, kv_work_queue* resault_buf):
    kv_base(),
    _db_name(db_name),
    _file_id(file_id),
    _data_file_name(data_file_name),
    _req_buf(request_buf),
    _res_buf(resault_buf)
{
    _module_name = "WORKER_";
    _module_name.append(tool::itostr(file_id));
    _idx = new kv_index();
    _idx->init(db_name, file_id, data_file_name);
    _idx->open_index(create);
}


kv_worker::~kv_worker()
{
    delete _idx;
}

void kv_worker::work()
{
    int fd = 0;
    kv_work* req = NULL;
    kv_work* res = NULL;
    while(1)
    {
        fd = _get_work(&req);
        if(fd == 0 || req == NULL) continue;

        if((req->head.flag & KV_WORK_FLAG_GET) == KV_WORK_FLAG_GET)
        {
            char* v = NULL;
            int re = _idx->get(&v, req->field1, req->head.field1_len);
            if(re < 0) res = _make_result(KV_WORK_FLAG_ERR, NULL,  0, NULL, 0);
            else       res = _make_result(KV_WORK_FLAG_SUC, v   , re, NULL, 0);
        }
        else if((req->head.flag & KV_WORK_FLAG_SET) == KV_WORK_FLAG_SET)
        {
            if(_idx->set(req->field1, req->head.field1_len, req->field2, req->head.field2_len) == -1) 
                 res = _make_result(KV_WORK_FLAG_ERR, NULL,  0, NULL, 0);
            else res = _make_result(KV_WORK_FLAG_SUC, NULL,  0, NULL, 0);
        }
        else if((req->head.flag & KV_WORK_FLAG_DEL) == KV_WORK_FLAG_DEL)
        {
            if(_idx->del(req->field1, req->head.field1_len) == -1) 
                 res = _make_result(KV_WORK_FLAG_ERR, NULL,  0, NULL, 0);
            else res = _make_result(KV_WORK_FLAG_SUC, NULL,  0, NULL, 0);
        }
        if(res == NULL) abort();
        _put_res(fd, res);
        res = NULL;
    }
}

int kv_worker::_get_work(kv_work** w)
{
    int fd = 0;
    if(_req_buf->lock_queue() == -1) goto req_buf_err;
    while(_req_buf->size() == 0) if(_req_buf->wait_work() == -1) goto req_buf_err;
    if((fd = _req_buf->get_work(w)) == -1) goto req_buf_err;
    if(_req_buf->unlock_queue() == -1) goto req_buf_err;
    
    return fd;
    req_buf_err:
    abort();
}

void kv_worker::_put_res(int fd, kv_work* w)
{
    if(_res_buf->lock_queue() == -1) goto res_map_err;
    if(_res_buf->put_work(fd, w) == -1) goto res_map_err;
    if(_res_buf->signal_waiter() == -1) goto res_map_err;
    if(_res_buf->unlock_queue() == -1) goto res_map_err;
    return;
    res_map_err:
    abort();
}

kv_work* kv_worker::_make_result(const int flag, char* const field1, const int len1, char* const field2, const int len2)
{
    kv_work* w = new_kv_work();
    if(w == NULL)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        abort();
    }
    w->head.flag = flag;
    w->head.field1_len = len1;
    w->head.field2_len = len2;
    w->field1 = field1;
    w->field2 = field2;
    w->field1_offset = 0;
    w->field2_offset = 0;
    w->head_offset = 0;
}

kv_work* kv_worker::_err_result(const std::string& err)
{
    kv_work* w = new_kv_work();
    if(w == NULL)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        abort();
    }
    w->head.flag = KV_WORK_FLAG_ERR;
    w->head.field1_len = err.length()+1;
    w->head.field2_len = 0;
    w->field2 = NULL;
    
    w->field1 = new char[w->head.field1_len];
    strncpy(w->field1, err.data(), w->head.field1_len);
    w->field1[err.length()] = '\0';
    
    w->field1_offset = 0;
    w->field2_offset = 0;
    w->head_offset = 0;
    return w;
}

}
