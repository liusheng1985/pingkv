/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_worker.h
 * Author: liusheng
 *
 * Created on 2016年7月12日, 下午2:02
 */

#ifndef KV_WORKER_H
#define KV_WORKER_H


#include "../tool/kv_base.h"
#include "../index/kv_index.h"
#include "kv_work_queue.h"

using std::cout;
using std::endl;
namespace pingkv
{
    
class kv_worker: public kv_base
{

private:
    int _file_id;
    std::string _db_name;
    std::string _data_file_name;
    
    kv_work_queue*  _req_buf;
    kv_work_queue*  _res_buf;
    
    kv_index* _idx;
    
    kv_work* _err_result(const string& err);
    kv_work* _make_result(const int flag, char* const field1, const int len1, char* const field2, const int len2);
    int _get_work(kv_work** w);
    void _put_res(int fd, kv_work* w);
    
public:
    kv_worker(const std::string& db_name, int file_id, const std::string& data_file_name, bool create, kv_work_queue* request_buf, kv_work_queue* resault_buf);
    virtual ~kv_worker();

    void work();

};

}
#endif /* KV_WORKER_H */

