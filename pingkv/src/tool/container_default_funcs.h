/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   container_default_func.h
 * Author: liusheng
 *
 * Created on 2016年5月31日, 上午10:51
 */

#ifndef CONTAINER_DEFAULT_FUNC_H
#define CONTAINER_DEFAULT_FUNC_H

#include <stdlib.h>
#include <assert.h>
#include "hash.h"

void* default_alloc(const size_t size);

void* default_free(const void* ptr);

template <typename T>
int default_compare(const T& data1, const T& data2);

template <typename T>
long default_hash(const T& data);


int int_compare(const int& data1, const int& data2);
long int_hash(const int& data);


#endif /* CONTAINER_DEFAULT_FUNC_H */

