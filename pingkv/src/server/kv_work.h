/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_pack.h
 * Author: liusheng
 *
 * Created on 2016年8月11日, 上午9:20
 */

#ifndef KV_WORK_H
#define KV_WORK_H

#include <stdlib.h>
#include <stdint.h>
#include "../conf/kv_conf.h"

namespace pingkv
{
#define KV_WORK_FLAG_REQ  0x1000
#define KV_WORK_FLAG_SUC  0x2000
#define KV_WORK_FLAG_ERR  0x4000
#define KV_WORK_FLAG_GET   0x100
#define KV_WORK_FLAG_SET   0x200
#define KV_WORK_FLAG_DEL   0x400
#define KV_WORK_FLAG_CMD   0x800

struct kv_work_head
{
    uint16_t flag;
    uint16_t field1_len;  // column1: key/error/command start from (char*)(&kv_work) + sizeof(kv_pack)
    uint16_t field2_len;  // column2:                   start from (char*)(&kv_work) + sizeof(kv_work) + col1_len
};

struct kv_work
{
    kv_work_head head;
    char* field1;
    char* field2;
    uint16_t head_offset;
    uint16_t field1_offset;
    uint16_t field2_offset;
};

kv_work* new_kv_work();

}
#endif /* KV_WORK_H */



















