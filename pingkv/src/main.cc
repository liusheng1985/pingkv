/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: liusheng
 *
 * Created on 2016年5月3日, 上午10:41
 */



#include "main.h"
#include "server/kv_server.h"


using namespace std;


void fill(char* p, char c, int len) {for(int i=0; i<len; i++) p[i] = c;}


int main(int argc, char** argv) 
{
    string file_name = "/opt/dev/lskv/testdb.conf";
    pingkv::kv_conf::single().read_conf(file_name);
    if(!pingkv::kv_conf::single().is_read_ok()) abort();
    int local_sid = pingkv::kv_conf::single().get_local_server_id();
    pingkv::kv_conf_data cd = pingkv::kv_conf::single().get_server_conf(local_sid);
    pingkv::kv_server s(cd);
    s.serv();
    
}


