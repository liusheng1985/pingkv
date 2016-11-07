/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_route.cc
 * Author: liusheng
 * 
 * Created on 2016年7月12日, 下午1:58
 */

#include "kv_route.h"

namespace pingkv
{
    
kv_route::kv_route(int server_count): _server_count(server_count){}
kv_route::~kv_route(){}

int kv_route::get_server_no(const char* k, int k_len)
{
    if(k == NULL) return -1;
    return hash_data(k, k_len)%_server_count;
}



}








