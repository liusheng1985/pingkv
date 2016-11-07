/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_base.h
 * Author: liusheng
 *
 * Created on 2016年7月12日, 下午9:25
 */

#ifndef KV_BASE_H
#define KV_BASE_H

#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <fcntl.h> 
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <queue>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>

#include "../err/kv_logger.h"
#include "../tool/hash.h"
#include "../tool/my_mutex.h"
#include "../tool/tool.h"
#include "../tool/llist.h"


using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::stack;
using std::queue;
using std::list;
using std::map;



namespace pingkv
{
class kv_base
{
protected:
    string _module_name;
public:
};
}
#endif /* KV_BASE_H */

