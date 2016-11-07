/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_route.h
 * Author: liusheng
 *
 * Created on 2016年7月12日, 下午1:58
 */

#ifndef KV_ROUTE_H
#define KV_ROUTE_H

#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "../tool/llist.h"
#include "../tool/hashtable.h"
#include "../tool/hash.h"
#include "boost/noncopyable.hpp"
#include "boost/bind.hpp"
#include "boost/function.hpp"

namespace pingkv
{

class kv_route: public boost::noncopyable
{
public:
private:
    int _server_count;
public:
    kv_route(int server_count);
    virtual ~kv_route();
    int get_server_no(const char* k, int k_len);
};


}

#endif /* KV_ROUTE_H */

