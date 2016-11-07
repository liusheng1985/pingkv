/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tool.cc
 * Author: liusheng
 * 
 * Created on 2016年6月7日, 下午3:24
 */

#include "tool.h"

namespace pingkv
{
tool::tool() {
}

tool::tool(const tool& orig) {
}

tool::~tool() {
}

std::string tool::itostr(int n)
{
    char s[20];
    sprintf(s, "%d", n);
    return std::string(s);
}

std::string tool::ltostr(long n)
{
    char s[20];
    sprintf(s, "%ld", n);
    return std::string(s);
}

std::string tool::dtostr(double n)
{
    char s[20];
    sprintf(s, "%lf", n);
    return std::string(s);
}

bool tool::pid_exists(pid_t pid)
{
    std::string pid_str = tool::ltostr(pid);
    DIR* dir = NULL;
    struct dirent* drt;
    if((dir = opendir("/proc")) == NULL) return -1;
    while((drt = readdir(dir)) != NULL) if(pid_str.compare(drt->d_name)==0) return true;
    closedir(dir);
    return false;
}

std::string tool::now()
{
    time_t t;
    time(&t);
    tm* m = localtime(&t);
    char s[20];
    sprintf(s, "%d-%s%d-%s%d %s%d:%s%d:%s%d",
            m->tm_year+1900, 
            m->tm_mon<=9?"0":"",m->tm_mon, 
            (m->tm_mday+1)<=9?"0":"",(m->tm_mday+1), 
            m->tm_hour<=9?"0":"",m->tm_hour,
            m->tm_min<=9?"0":"",m->tm_min,
            m->tm_sec<=9?"0":"",m->tm_sec);
    return std::string(s);
}

time_t tool::now_sec()
{
    time_t t;
    time(&t);
    return t;
}

__suseconds_t tool::now_microsec()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_usec + t.tv_sec*1000*1000);
}

std::string tid_to_str(pthread_t tid)
{
    char s[24];
    memset(s, '\0', 24);
    sprintf(s, "%ld", (unsigned long)tid);
    return std::string(s);
}


}