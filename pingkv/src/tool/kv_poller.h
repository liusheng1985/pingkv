/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   poller.h
 * Author: liusheng
 *
 * Created on 2016年7月18日, 上午11:33
 */

#ifndef POLLER_H
#define POLLER_H

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <assert.h>
#include <map>
#include <iostream>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "my_mutex.h"
#include "tool.h"
using std::cout;
using std::endl;
namespace pingkv
{

class kv_poller: public boost::noncopyable
{
public:
    struct poller_arg;
    typedef boost::function<void(int fd, void* arg)> poller_cb_t;
    struct poller_arg
    {
        int fd;
        void* cust_arg;
        epoll_event ev;
        poller_cb_t rcb;
        poller_cb_t wcb;
    };
    
private:
    int _ep;
    std::map<int, poller_arg*> _emap;
    epoll_event _evs[BUFSIZ*10];
    
    my_mutex _x;
    
    int _no_block(int fd);
    int _check_epoll_err(epoll_event* ee);
    
    int _regist(int fd, int ev, poller_cb_t rcb, poller_cb_t wcb, void* cust_arg);
    int _unregist(int fd);
    
public:
    kv_poller();
    virtual ~kv_poller();

    int regist_r (int fd, poller_cb_t cb, void* cust_arg);
    int regist_w (int fd, poller_cb_t cb, void* cust_arg);
    int regist_rw(int fd, poller_cb_t rcb, poller_cb_t wcb, void* cust_arg);
    int unregist (int fd);
    int loop();
    
};
}
#endif /* POLLER_H */

