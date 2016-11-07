/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "container_default_funcs.h"

void* default_alloc(const size_t size)
{
    void* ptr = malloc(size);
    assert(ptr);
    return ptr;
}

void* default_free(const void* ptr)
{
    void* p = (void*)ptr;
    if(p != NULL)
    {
        free(p);
        p = NULL;
    }
    return p;
}

template <typename T>
int default_compare(const T& data1, const T& data2)
{
    T d1 = data1;
    T d2 = data2;
    if(d1 == d2) return 0;
    if(d1 >  d2) return 1;
    else return -1;
}

template <typename T>
long default_hash(const T& data) {return hash_data(&data, sizeof(T));}


int int_compare(const int& data1, const int& data2)
{
    if(data1 == data2) return 0;
    if(data1 >  data2) return 1;
    else return -1;
}

long int_hash(const int& data) {return hash_data(&data, sizeof(int));}

