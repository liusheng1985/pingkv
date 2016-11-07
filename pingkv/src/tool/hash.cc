/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "hash.h"


long hash_data(const void* data, const int len)
{
    long hash = len;
    int i=0;
    for(char* c=(char*)data; len>i++; hash=hash*1313+(*(c++)));
    return hash;
}

long hash_long(long l) {return (long)(l * LSHASH_S_64);}
