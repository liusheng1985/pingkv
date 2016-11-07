/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "data_file.h"
namespace pingkv
{

int data_file::_row_count(const block_id_t block_id)
{
    data_block* blk;
    if(_read_block(&blk, block_id) == -1) return -1;
    return _row_count(blk);
}

int data_file::_row_count(const data_block* block) {return block->head.record_count;}




int data_file::_row_add_record(data_block* b, row_len_t len, bool chained)
{
    record_index* idx = (record_index*)b->data;
    short last_offset = sizeof(b->data);
    if(b->head.record_count>0)
    {
        idx = idx + (b->head.record_count - 1);
        last_offset = idx->offset;
        ++idx;
    }
    b->head.record_count += 1;
    idx->flags = 0;
    if(chained) idx->flags = PINGKV_DBF_ROW_SET_CHAINED(idx->flags);
    idx->len = len;
    idx->next_block_id = 0;
    idx->next_row_idx = 0;
    idx->offset = chained ? (b->head.record_count*sizeof(record_index)) : last_offset - len;
    return b->head.record_count;
}

int data_file::_row_get_record(record_index** record, const data_block* b, row_idx_t idx)
{
    if(idx < 1 || idx > b->head.record_count)
    {
        char e[BUFSIZ];
        sprintf(e, "iinvalid row index: file %s block %d row %d\n", _path_name.c_str(), b->head.block_id, idx);
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
        return -1;
    }
    
    *record = (record_index*)b->data;
    *record = *record + (idx-1);
    return 0;
}

int data_file::_row_get_pos(char** pos, data_block* b, row_idx_t idx)
{
    if(idx > b->head.record_count)
    {
        char e[BUFSIZ];
        sprintf(e, "iinvalid row index: file %s block %d row %d\n", _path_name.c_str(), b->head.block_id, idx);
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
        return -1;
    }
    record_index* ri = (record_index*)b->data;
    *pos = b->data + (ri + idx)->offset;
    return 0;
}


int data_file::_read_row(char** row, const block_id_t block_id, const row_idx_t idx)
{
    *row = NULL;
    row_idx_t curr_idx = idx;
    block_id_t curr_block_id = block_id;
    data_block* curr_block = NULL;
    record_index* curr_record = NULL;
    row_len_t row_len = 0;
    
    if(_read_block(&curr_block, curr_block_id) == -1) return -1;
    if(_row_get_record(&curr_record, curr_block, idx) == -1) return -1;
    row_len = curr_record->len;
    
    if(PINGKV_DBF_ROW_DELETED(curr_record->flags) == 1) return 0;
    if(PINGKV_DBF_ROW_MIGRATED(curr_record->flags) == 1) return _read_row(row, curr_record->next_block_id, curr_record->next_row_idx);
    
    if((*row = new char[row_len]) == NULL)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }

    if(PINGKV_DBF_ROW_CHAINED(curr_record->flags) == 1)
    {
        row_len_t read_len = 0;
        row_len_t reamin_len = row_len;
        char* p = *row;
        
        while(1)
        {
            read_len = _row_get_len_in_block(curr_block, curr_idx);
            memcpy(p, (curr_block->data + curr_record->offset), read_len);
            reamin_len -= read_len;
            p += read_len;
            
            if(reamin_len > 0)
            {
                curr_block_id = curr_record->next_block_id;
                curr_idx = curr_record->next_row_idx;
                if(_read_block(&curr_block, curr_block_id) == -1) return -1;
                if(_row_get_record(&curr_record, curr_block, curr_idx) == -1) return -1;
            }
            else break;
        }
    }
    else memcpy(*row, (curr_block->data + curr_record->offset), curr_record->len);
    return row_len;
}


int data_file::_write_row(short* row_idx, const block_id_t block_id, const char* data, row_len_t len)
{
    data_block* block = NULL;
    if(_read_block(&block, block_id) == -1) return -1;
    return _write_row(row_idx, block, data, len);
}

int data_file::_write_row(short* row_idx, data_block* block, const char* data, row_len_t len)
{
    record_index* record = NULL;
    row_len_t fre = _block_free_bytes(block);
    row_len_t write_len = fre >= len ? len : fre;
    row_len_t chained_len = fre >= len ? 0 : (len - fre);
    row_idx_t idx = _row_add_record(block, len, (fre < len));
    if(idx == -1) return -1;
    if(_row_get_record(&record, block, idx) == -1) return -1;
    char* pos = block->data + record->offset;
    
    
    if(chained_len > 0)
    {
        data_block* chained_block = NULL;
        if(_find_block_can_write(&chained_block) == -1) return -1;
        row_idx_t chained_row_idx = 0;
        if(_write_row(&chained_row_idx, chained_block, data+write_len, len - write_len) == -1) return -1;
        record->next_block_id = chained_block->head.block_id;
        record->next_row_idx = chained_row_idx;
    }
    memcpy(pos, data, write_len);
    *row_idx = idx;
    return 0;
}


int data_file::_modify_row(const block_id_t block_id, const row_idx_t idx, const char* row, const row_len_t len)
{
    record_index* record = NULL;
    data_block* block = NULL;
    if(_read_block(&block, block_id) == -1) return -1;
    if(_row_get_record(&record, block, idx) == -1) return -1;
    
    if(PINGKV_DBF_ROW_DELETED(record->flags) == 1) return -1;
    if(PINGKV_DBF_ROW_MIGRATED(record->flags) == 1) return _modify_row(record->next_block_id, record->next_row_idx, row, len);
    if(PINGKV_DBF_ROW_CHAINED(record->flags) == 1 || len > record->len)
    {
        record->flags = PINGKV_DBF_ROW_SET_MIGRATED(record->flags);
        
        data_block* new_block = NULL;
        if(_find_block_can_write(&new_block) == -1) return -1;
        short new_row_idx = 0;
        if(_write_row(&new_row_idx, new_block, row, len) == -1) return -1;
        record->next_block_id = new_block->head.block_id;
        record->next_row_idx = new_row_idx;
    }
    else
    {
        memset(block->data+record->offset, 0, record->len);
        memcpy(block->data+record->offset, row, len);
    }
    return 0;
}


int data_file::_delete_row(const block_id_t block_id, const row_idx_t idx)
{
    record_index* record = NULL;
    data_block* block = NULL;
    if(_read_block(&block, block_id) == -1) return -1;
    if(_row_get_record(&record, block, idx) == -1) return -1;
    
    record->flags = PINGKV_DBF_ROW_SET_DELETED(record->flags);
    return 0;
}


int data_file::_row_get_len_in_block(data_block* b, row_idx_t idx)
{
    record_index* curr_record = NULL;
    record_index* next_record = NULL;
    int curr_offset = -1;
    int next_offset = -1;
    
    if(_row_get_record(&curr_record, b, idx) == -1) return -1;
    if(idx > 1) if(_row_get_record(&next_record, b, idx-1) == -1) return -1;
    
    curr_offset = curr_record->offset;
    next_offset = idx == 1 ? sizeof(b->data) : next_record->offset;
    
    if(next_offset - curr_offset > curr_record->len) return curr_record->len;
    else return next_offset - curr_offset;
}


}











