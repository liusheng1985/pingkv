/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tool.h
 * Author: liusheng
 *
 * Created on 2016年6月7日, 下午3:24
 */

#ifndef TOOL_H
#define TOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/time.h>
#include <string>


namespace pingkv
{
class tool
{
public:
    tool();
    tool(const tool& orig);
    virtual ~tool();
    
    static std::string itostr(int n);
    static std::string ltostr(long n);
    static std::string dtostr(double n);
    static bool pid_exists(pid_t pid);
    static std::string now();
    static time_t now_sec();
    static __suseconds_t now_microsec();
    static std::string tid_to_str(pthread_t tid);
private:

};
}
#endif /* TOOL_H */

