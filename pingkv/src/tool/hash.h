/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   hash.h
 * Author: liusheng
 *
 * Created on 2016年5月3日, 上午11:06
 */

#ifndef LSHASH_H
#define LSHASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 


#define LSHASH_S_32 2654435769
#define LSHASH_S_64 5534023222112865484



long hash_data(const void* data, const int len);
long hash_long(long l);


#endif /* LSHASH_H */

