/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lsmutex.h
 * Author: liusheng
 *
 * Created on 2016年5月26日, 上午9:38
 */

#ifndef LSMUTEX_H
#define LSMUTEX_H
#include <stdlib.h>
#include <unistd.h>
#include <boost/noncopyable.hpp>


class my_mutex: public boost::noncopyable
{
public:
    my_mutex(){pthread_mutex_init(&_mutex, NULL);}
    ~my_mutex(){pthread_mutex_destroy(&_mutex);}
    void lock() {pthread_mutex_lock(&_mutex);}
    void unlock() {pthread_mutex_unlock(&_mutex);}
private:
    pthread_mutex_t _mutex;
};


class my_mutex_locker: public boost::noncopyable
{
public:
    explicit my_mutex_locker(my_mutex& lk): _lk(lk){_lk.lock();}
    ~my_mutex_locker(){_lk.unlock();}
private:
    my_mutex& _lk;
};


#endif /* LSMUTEX_H */

