/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   poller.cc
 * Author: liusheng
 * 
 * Created on 2016年7月18日, 上午11:33
 */

#include "kv_poller.h"
namespace pingkv
{
kv_poller::kv_poller() {
    if((_ep = epoll_create1(0)) < 0)
    {
        fprintf(stderr, "create epoll error: %s\n    file %s function %s, line %d\n", strerror(errno), __FILE__, __func__,__LINE__);
        exit(1);
    }
}

kv_poller::~kv_poller() {
    my_mutex_locker lock(_x);
    for(std::map<int, poller_arg*>::iterator it=_emap.begin(); it!=_emap.end(); it++) _unregist(it->first);
    _emap.clear();
    close(_ep);
}

int kv_poller::_regist(int fd, int ev, poller_cb_t rcb, poller_cb_t wcb, void* cust_arg)
{
//    if(_no_block(fd) == -1) return -1;
    _emap[fd] = new poller_arg();
    if(_emap[fd] == NULL)
    {
        fprintf(stderr, "regist event error: %s\n    file %s function %s, line %d\n", strerror(errno), __FILE__, __func__,__LINE__);
        return -1;
    }
    
    poller_arg* pa = _emap[fd];
    pa->cust_arg = cust_arg;
    pa->fd = fd;
    pa->ev.events = ev|EPOLLET;
    pa->ev.data.ptr = pa;
    pa->rcb = rcb;
    pa->wcb = wcb;
    if(epoll_ctl(_ep, EPOLL_CTL_ADD, fd, &pa->ev) == -1)
    {
        fprintf(stderr, "regist event error: %s\n    file %s function %s, line %d\n", strerror(errno), __FILE__, __func__,__LINE__);
        return -1;
    }
    return 0;
}

int kv_poller::regist_r (int fd, poller_cb_t cb, void* cust_arg) {my_mutex_locker lock(_x);return _regist(fd, EPOLLIN , cb, NULL, cust_arg);}
int kv_poller::regist_w (int fd, poller_cb_t cb, void* cust_arg) {my_mutex_locker lock(_x);return _regist(fd, EPOLLOUT, NULL, cb, cust_arg);}
int kv_poller::regist_rw(int fd, poller_cb_t rcb, poller_cb_t wcb, void* cust_arg) 
{
    my_mutex_locker lock(_x);
    if(_regist(fd, EPOLLIN|EPOLLOUT , rcb, wcb, cust_arg) == -1) return -1;
    return 0;
}

int kv_poller::_unregist(int fd)
{
    if(epoll_ctl(_ep, EPOLL_CTL_DEL, fd, &_emap[fd]->ev) == -1)
    {
        fprintf(stderr, "ubregist event error: %s\n    file %s function %s, line %d\n", strerror(errno), __FILE__, __func__,__LINE__);
        return -1;
    }
    delete _emap[fd];
    _emap.erase(fd);
}

//EPOLL_CTL_DEL
int kv_poller::unregist (int fd)
{
    my_mutex_locker lock(_x);
    return _unregist(fd);
}

int kv_poller::loop()
{
    int n;
    while(1)
    {
        {
            my_mutex_locker lock(_x);
            n = epoll_wait(_ep, _evs, BUFSIZ*10, -1);
        }
        if(n == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }
            fprintf(stderr, "listening events error: %s\n    file %s function %s, line %d\n", strerror(errno), __FILE__, __func__,__LINE__);
            return -1;
        }
        for(int i=0; i<n; i++)
        {
            poller_arg* pa = (poller_arg*)_evs[i].data.ptr;
            if(_check_epoll_err(&(_evs[i])) < 0)
            {
                _unregist(pa->fd);
                continue;
            }
            if((pa->ev.events&EPOLLIN ) == EPOLLIN ) pa->rcb(pa->fd, pa->cust_arg);
            if((pa->ev.events&EPOLLOUT) == EPOLLOUT) pa->wcb(pa->fd, pa->cust_arg);
        }
    }
}

int kv_poller::_no_block(int fd)
{
    int fl = 0;
    int r  = 0;
    if((fl = fcntl(fd, F_GETFL, 0)) < 0) 
    {
        fprintf(stderr, "making fd nonblock error: %s\n    file %s function %s, line %d\n", strerror(errno), __FILE__, __func__,__LINE__);
        exit(1);
    }
    if((r  = fcntl(fd, F_SETFL, fl|O_NONBLOCK)) < 0)
    {
        fprintf(stderr, "making fd nonblock error: %s\n    file %s function %s, line %d\n", strerror(errno), __FILE__, __func__,__LINE__);
        exit(1);
    }
    return 0;
}

int kv_poller::_check_epoll_err(epoll_event* ee)
{
    if(ee == NULL)
    {
        fprintf(stderr, "checking event error: file %s function %s, line %d\n", __FILE__, __func__,__LINE__);
        return -1;
    }
    if( (ee->events & EPOLLERR) == EPOLLERR || (ee->events & EPOLLHUP) == EPOLLHUP)
    {
        fprintf(stderr, "checking event error: %d\n    file %s function %s, line %d\n", ((ee->events & EPOLLERR) | (ee->events & EPOLLHUP)), __FILE__, __func__,__LINE__);
        return -1;
    }
    return 0;
}



}