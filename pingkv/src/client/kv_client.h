/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_client.h
 * Author: liusheng
 *
 * Created on 2016年7月12日, 下午1:58
 */

#ifndef KV_CLIENT_H
#define KV_CLIENT_H

#include <boost/noncopyable.hpp>
#include "kv_route.h"
#include "../conf/kv_conf.h"
#include "../server/kv_work.h"
#include <vector>

#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <fcntl.h> 
#include <sys/epoll.h> 
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>


namespace pingkv
{

class kv_client: public boost::noncopyable
{
public:
    
private:
    int _server_count;
    kv_route* _r;
    std::vector<std::pair<string,int> > _servers;
    int* _socks;
    
    int _conn(const char* ip, int port);
    int _send_work(const kv_work* work, int server_no);
    int _recv_result(kv_work* work, int server_no);
    
public:
    kv_client(std::vector<std::pair<string,int> >& servers);
    virtual ~kv_client();
    
    
    int init();
    int get(char** v, int* v_len, const char* k, const int k_len);
    int set(const char* k, const int k_len, const char* v, const int v_len);
    int del(const char* k, const int k_len);
    
};



}
#endif /* KV_CLIENT_H */

