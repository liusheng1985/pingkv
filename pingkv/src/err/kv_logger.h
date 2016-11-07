/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   err.h
 * Author: liusheng
 *
 * Created on 2016年6月6日, 下午5:01
 */

#ifndef ERR_H
#define ERR_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <iostream>
#include <map>
#include <boost/noncopyable.hpp>
#include "../tool/tool.h"
#include "../tool/my_mutex.h"



namespace pingkv
{

class kv_logger: public boost::noncopyable
{
private:
    my_mutex _x;
    void _make_log_txt(FILE* f, const std::string& file, const std::string& func, const int line, const char* module_name, const char* err_str);
    kv_logger();
    static kv_logger* _one;
public:
    static kv_logger* get();
    void errlog (const char* str);
    void infolog(const char* str);
    int errlog_fmt (const std::string& file, const std::string& func, const int line, const char* module_name, const char* err_str);
    int oserrlog_fmt(const std::string& file, const std::string& func, const int line, const char* module_name, const int erno);
//    void oserrlog_fmt(const char* file, const char* func, const int line, const char* module_name, const int erno);
    int infolog_fmt(const std::string& file, const std::string& func, const int line, const char* module_name, const char* err_str);
};



}
#endif /* ERR_H */

