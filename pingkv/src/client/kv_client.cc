/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_client.cc
 * Author: liusheng
 * 
 * Created on 2016年7月12日, 下午1:58
 */

#include "kv_client.h"

namespace pingkv
{
kv_client::kv_client(std::vector<std::pair<string,int> >& servers)
{
    _server_count = servers.size();
    _servers = servers;
}

kv_client::~kv_client() {}


int kv_client::_conn(const char* ip, int port)
{
    int sock = 0;
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
    // connect
    if(connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) return -1;
    return sock;
}

int kv_client::_send_work(const kv_work* work, int server_no)
{
    if(send(_socks[server_no], const_cast<kv_work_head*>(&work->head), sizeof(kv_work_head), 0) != sizeof(kv_work_head)) return -1;
    if(work->head.field1_len > 0) 
    {
        int re = send(_socks[server_no], work->field1, work->head.field1_len, 0);
        if(re != work->head.field1_len) return -1;       
    }
    if(work->head.field2_len > 0 && send(_socks[server_no], work->field2, work->head.field2_len, 0) != work->head.field2_len) return -1;
    return 0;
}

int kv_client::_recv_result(kv_work* work, int server_no)
{
    if(recv(_socks[server_no], const_cast<kv_work_head*>(&work->head), sizeof(kv_work_head), 0) != sizeof(kv_work_head)) return -1;
    work->field1 = new char[work->head.field1_len];
    if(work->head.field1_len > 0 && recv(_socks[server_no], work->field1, work->head.field1_len, 0) != work->head.field1_len) return -1;
    if(work->head.field2_len > 0 && recv(_socks[server_no], work->field2, work->head.field2_len, 0) != work->head.field2_len) return -1;
    return 0;
}


int kv_client::init()
{
    int i=0;
    int sock = 0;
    _r = new kv_route(_server_count);
    _socks = new int[_server_count];
    for(std::vector<std::pair<string,int> >::iterator it=_servers.begin(); it!=_servers.end(); it++)
    {
        if((sock = _conn(it->first.data(), it->second)) == -1) 
            return -1;
        _socks[i] = sock;;
        i++;
    }
    return 0;
}

int kv_client::get(char** v, int* v_len, const char* k, const int k_len)
{
    kv_work* w = new_kv_work();
    w->field1 = const_cast<char*>(k);
    w->head.field1_len = k_len;
    w->head.flag = KV_WORK_FLAG_GET|KV_WORK_FLAG_REQ;

    int server_no = _r->get_server_no(k, k_len);
    if(_send_work(w, server_no) == -1) return -1;
    if(_recv_result(w, server_no) == -1) return -1;
    *v_len = w->head.field1_len;
    *v = new char[w->head.field1_len];
    strcpy(*v, w->field1);
    delete w;
    return (w->head.flag & KV_WORK_FLAG_SUC == KV_WORK_FLAG_SUC);
}


int kv_client::set(const char* k, const int k_len, const char* v, const int v_len)
{
    kv_work* w = new_kv_work();
    w->field1 = const_cast<char*>(k);
    w->head.field1_len = k_len;
    w->field2 = const_cast<char*>(v);
    w->head.field2_len = v_len;
    w->head.flag = KV_WORK_FLAG_SET|KV_WORK_FLAG_REQ;

    int server_no = _r->get_server_no(k, k_len);
    if(_send_work(w, server_no) == -1) return -1;
    if(_recv_result(w, server_no) == -1) return -1;
    delete w;
    return (w->head.flag & KV_WORK_FLAG_SUC == KV_WORK_FLAG_SUC);
}

int kv_client::del(const char* k, const int k_len)
{
    kv_work* w = new_kv_work();
    w->field1 = const_cast<char*>(k);
    w->head.field1_len = k_len;
    w->head.flag = KV_WORK_FLAG_GET|KV_WORK_FLAG_REQ;

    int server_no = _r->get_server_no(k, k_len);
    if(_send_work(w, server_no) == -1) return -1;
    if(_recv_result(w, server_no) == -1) return -1;
    delete w;
    return (w->head.flag & KV_WORK_FLAG_SUC == KV_WORK_FLAG_SUC);
}


}


