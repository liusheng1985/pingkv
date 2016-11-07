/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   err.cc
 * Author: liusheng
 * 
 * Created on 2016年6月6日, 下午5:01
 */

#include "kv_logger.h"

namespace pingkv
{
kv_logger* kv_logger::_one = NULL;
kv_logger::kv_logger() {}

void kv_logger::errlog (const char* str) {my_mutex_locker lock(_x);fprintf(stderr, str);}
void kv_logger::infolog(const char* str) {my_mutex_locker lock(_x);fprintf(stdout, str);}
kv_logger* kv_logger::get() {if(_one == NULL) _one = new kv_logger(); return _one;}

int kv_logger::infolog_fmt (const std::string& file, const std::string& func, const int line, const char* module_name, const char* err_str) {_make_log_txt(stdout, file, func, line, module_name, err_str);return 0;}
int kv_logger::errlog_fmt  (const std::string& file, const std::string& func, const int line, const char* module_name, const char* err_str) {_make_log_txt(stderr, file, func, line, module_name, err_str);return -1;}
int kv_logger::oserrlog_fmt(const std::string& file, const std::string& func, const int line, const char* module_name, const int   erno)
{
    char e[BUFSIZ];
    sprintf(e, "OS level error: %d(%s)", erno, strerror(erno));
    _make_log_txt(stderr, file, func, line, module_name, e);
    return -1;
}

void kv_logger::_make_log_txt(FILE* f, const std::string& file, const std::string& func, const int line, const char* module_name, const char* err_str)
{
    std::string e;
    char head[BUFSIZ];
    char text[BUFSIZ];
    memset(head, '0', BUFSIZ);
    memset(text, '0', BUFSIZ);
    sprintf(head, "[%s] [%s] [%s()+%d] %s: ", tool::now().c_str(), module_name, func.c_str(), line, f==stderr?"ERROR":"INFO");
    snprintf(text, BUFSIZ, "%s", err_str);
    if(strlen(err_str) > BUFSIZ);
    e.append(head);
    e.append(text);
    if(strlen(err_str) > BUFSIZ) e.append("...\n");
    
    my_mutex_locker lock(_x);
    fprintf(f, e.c_str());
}


}