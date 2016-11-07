/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "kv_work.h"

namespace pingkv
{
    kv_work* new_kv_work()
    {
        kv_work* w = new kv_work;
        if(w == NULL) return NULL;
        w->field1 = NULL;
        w->field2 = NULL;
        w->field1_offset = 0;
        w->field2_offset = 0;
        w->head_offset = 0;
        w->head.field1_len = 0;
        w->head.field2_len = 0;
        w->head.flag = 0;
        return w;
    }
}