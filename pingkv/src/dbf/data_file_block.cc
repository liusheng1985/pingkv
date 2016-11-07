/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "data_file.h"

namespace pingkv
{
    
int data_file::_format_block(data_block* p, const block_id_t block_id)
{
    p->head.block_id = block_id;
    p->head.record_count = 0;
    if(_block_checksum(p) == -1) return -1;
    p->tail.checksum = p->head.checksum;
    return 0;
}

int data_file::_read_block(data_block** p, const block_id_t block_id)
{
    *p = _cache[block_id];
    if(*p == NULL)
    {
        if((*p = (data_block*)mmap(NULL, sizeof(data_block), PROT_WRITE, MAP_SHARED, _fd, (block_id-1)*PINGKV_DBF_BLOCKSIZE)) == MAP_FAILED)
        {
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            return -1;
        }
        /* 
         * TEMPORARY DISABLE 
         * 
        if((*p)->head.checksum != (*p)->tail.checksum)
        {
            char e[BUFSIZ];
            sprintf(e, errmsg::get_static_err_map()[errmsg::corrupt_block_erno].c_str(), block_id, _path_name.c_str());
            _err.push_back(e);
            return -1;
        }
        
        _block_checksum(*p);
        */
        _cache[block_id] = *p;
    }
    return 0;
}

int data_file::_flush_out_block(const block_id_t block_id)
{
    if(block_id < 2 || block_id > _file_head->info.block_count) 
    {
        char e[BUFSIZ];
        sprintf(e, "invalid block id: file %s block %d\n", _path_name.c_str(), block_id);
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
        return -1;
    }
    data_block* p = _cache[block_id];
    if(p)
    {
        return _flush_out_block(p);
    }
}

int data_file::_flush_out_block(data_block* p)
{
    if(p) 
    {
        block_id_t block_id = p->head.block_id;
        p->tail.checksum = p->head.checksum;
        if(munmap(p, PINGKV_DBF_BLOCKSIZE) == -1)
        {
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            return -1;
        }
        else 
        {
            _cache.erase(block_id);
            return 0;
        };
    }
    return 0;
}

int data_file::_flush_block(const block_id_t block_id)
{
    data_block* p = _cache[block_id];
    if(!p) return -1;
    return _flush_block(p);
}

int data_file::_flush_block(data_block* block)
{
    block->tail.checksum = block->head.checksum;
    if(msync(block, PINGKV_DBF_BLOCKSIZE, MS_SYNC) == -1)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    return 0;
}

int data_file::_block_free_bytes(const block_id_t block_id)
{
    data_block* p = NULL;
    if(_read_block(&p, block_id) == -1) return -1;
    return _block_free_bytes(p);
}

int data_file::block_free_bytes(const block_id_t block_id)
{
    return _block_free_bytes(block_id);
}


int data_file::_block_free_bytes(const data_block* block)
{
    record_index* last_record = NULL;
    if(block->head.record_count == 0) return sizeof(block->data) - sizeof(record_index);
    if(_row_get_record(&last_record, block, block->head.record_count) == -1) return -1;
    if(last_record == NULL) return sizeof(block->data - sizeof(record_index));
    row_len_t record_size = (block->head.record_count + 1) * sizeof(record_index); // include a new record_index
    if(last_record->offset <= record_size) return 0;
    return last_record->offset - record_size;
}



int data_file::_block_checksum(data_block* block)
{
    block->head.checksum = rand();
    return 0;
}

int data_file::_find_block_can_write(data_block** block)
{
    row_len_t fre = 0;
    if(_file_head->info.used_block_count == _file_head->info.block_count) _extend_file(128);
    if(_read_block(block, _file_head->info.used_block_count) == -1) return -1;
    if((fre = _block_free_bytes(*block)) == -1) return -1;
    while(fre == 0)
    {
        _file_head->info.used_block_count += 1;
        _flush_file_head();
        if(_read_block(block, _file_head->info.used_block_count) == -1) return -1;
        if((fre = _block_free_bytes(*block)) == -1) return -1;
    }
    return fre;
}

int data_file::_flush_out_cache()
{
    while(_cache.size() > 0) if(_flush_out_block(_cache.begin()->second)==-1) return -1;
    _cache.clear();
    return 0;
}

}