/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   data_file.cc
 * Author: liusheng
 * 
 * Created on 2016年6月3日, 上午11:06
 */

#include "data_file.h"

namespace pingkv
{
data_file::data_file():_file_head(NULL), _init_ok(false),_fd(-1) 
{
    _cache = std::map<int,data_block*>();
}

data_file::~data_file() 
{
    _flush_out_cache();
    if(_fd != -1) close(_fd);
}

int data_file::block_count() const
{
    if(_init_ok) return _file_head->info.block_count;
    return -1;
}

int data_file::used_block_count() const
{
    if(_init_ok) return _file_head->info.used_block_count;
    return -1;
}


int data_file::empty_block_space()
{
    data_block* b;
    return sizeof(b->data) - sizeof(record_index);
}

int data_file::create_file(const string& dbname, int file_id, const string& path_name)
{
    char* s = (char*)dbname.substr(0, 15).c_str();
    if((_fd = open(path_name.c_str(), O_RDWR|O_CREAT)) == -1) 
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        goto err;
    }
    if(fchmod(_fd, S_IRUSR|S_IWUSR|S_IRGRP) == -1)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        goto err;
    }
    // write file head
    file_head fh;
    fh.info.curr_pid = getpid();
    fh.info.file_id = file_id;
    strncpy(fh.info.db_name, s, 15);
    fh.info.block_size = PINGKV_DBF_BLOCKSIZE;
    fh.info.block_count = 1;
    fh.info.used_block_count = 1;
    _path_name = path_name;
    
    if(write(_fd, &fh, sizeof(file_head)) == -1) 
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        goto err;
    }
    
    if(_read_file_head() == -1) goto err;
    if(_extend_file(128) == -1) goto err;
    _file_head->info.used_block_count = 2;
    if(_flush_file_head() == -1) goto err;
    
    _module_name = "DATAFILE_";
    _module_name.append(tool::itostr(_file_head->info.file_id));
    _init_ok = true;
    return 0;
    
    err:
    char e[BUFSIZ];
    sprintf(e, "create datafile %s error.\n", path_name.c_str());
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}

int data_file::exists(const string& path_name)
{
    if((_fd = open(path_name.c_str(), O_RDWR)) == -1) if(errno != 2)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    if(errno == 2) return 0;
    close(_fd);
    return 1;
}

int data_file::open_file(const string& path_name)
{
    _path_name = path_name;
    
    if((_fd = open(path_name.c_str(), O_RDWR|O_EXCL)) == -1)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        goto err;
    }
    
    // read file head
    
    if(_read_file_head() == -1) goto err;
    
    
    if(_file_head->info.curr_pid != getpid() && tool::pid_exists(_file_head->info.curr_pid))
    {
        char s[BUFSIZ];
        sprintf(s, "datafile %s locked by process %d.\n", path_name.c_str(), _file_head->info.curr_pid);
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), s);
        goto err;
    }
    
    _file_head->info.curr_pid = getpid();
    _flush_file_head();
    
    _module_name = "DATAFILE_";
    _module_name.append(tool::itostr(_file_head->info.file_id));
    _init_ok = true;
    return 0;
    
    err:
    char e[BUFSIZ];
    sprintf(e, "open datafile %s error.\n", path_name.c_str());
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}


int data_file::write_row(block_id_t* block_id, row_idx_t* row_idx, const char* row, const row_len_t len)
{
    int fre = 0;
    data_block* b = NULL;
    if((fre = _find_block_can_write(&b)) == -1) goto err;
    *block_id = b->head.block_id;
    if(_write_row(row_idx, b, row, len) == -1) goto err;
    return 0;
    
    err:
    char e[BUFSIZ];
    sprintf(e, "write a row in datafile %s block %d error.\n", _path_name.c_str(), b->head.block_id);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}

int data_file::write_row_new_block(block_id_t* block_id, const char* row, const row_len_t len)
{
    data_block* b = NULL;
    row_idx_t row_idx = 0;
    if(_file_head->info.used_block_count == _file_head->info.block_count) _extend_file(128);
    _file_head->info.used_block_count += 1;
    _flush_file_head();
    
    if(_read_block(&b, _file_head->info.used_block_count) == -1) goto err;
    *block_id = b->head.block_id;
    if(_write_row(&row_idx, b, row, len) == -1) goto err;
    return 0;
    
    err:
    char e[BUFSIZ];
    sprintf(e, "write a row in datafile %s block %d error.\n", _path_name.c_str(), b->head.block_id);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}


int data_file::delete_row(const block_id_t block_id, const row_idx_t row_idx)
{
    if(_delete_row(block_id, row_idx) == -1) return -1;
    return 0;
    
    err:
    char e[BUFSIZ];
    sprintf(e, "delete datafile %s block %d row %d error.\n", _path_name.c_str(), block_id, row_idx);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}

int data_file::read_row(char** row, const block_id_t block_id, const row_idx_t row_idx)
{
    int re = 0;
    if((re = _read_row(row, block_id, row_idx)) == -1) goto err;
    
    return re;
    err:
    char e[BUFSIZ];
    sprintf(e, "read datafile %s block %d row %d error.\n", _path_name.c_str(), block_id, row_idx);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
    
}

int data_file::modify_row(const block_id_t block_id, const row_idx_t row_idx, const char* row, const row_len_t len)
{
    return _modify_row(block_id, row_idx, row, len);
    
    err:
    char e[BUFSIZ];
    sprintf(e, "modify datafile %s block %d row %d error.\n", _path_name.c_str(), block_id, row_idx);
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    return -1;
}


int data_file::flush_blocks()
{
    return _flush_out_cache();
}
 


int data_file::_extend_file(int blocks)
{
    if(lseek(_fd, 0, SEEK_END) == -1)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    
    if(blocks <= 0) blocks = 128;
    block_id_t block_id = _file_head->info.block_count;
    data_block blk;
    int re = 0;
    for(int i=1; i<=blocks; i++)
    {
        if(_format_block(&blk, block_id+i) < 0) return -1;
        if(write(_fd, &blk, sizeof(data_block)) == -1)
        {
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            return -1;
        }
        _file_head->info.block_count += 1;
        re ++;
    }
    
    return re;
}


int data_file::_read_file_head()
{
    _file_head = (file_head*)mmap(NULL, sizeof(file_head), PROT_WRITE, MAP_SHARED, _fd, 0);
    if(_file_head == MAP_FAILED)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    return 0;
}

int data_file::_flush_file_head()
{
    if((msync(_file_head, sizeof(file_head), MS_SYNC)) == -1)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    return 0;
}














}








