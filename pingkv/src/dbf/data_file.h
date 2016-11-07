/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   data_file.h
 * Author: liusheng
 *
 * Created on 2016年6月3日, 上午11:06
 */

#ifndef DATA_FILE_H
#define DATA_FILE_H


#include "../tool/kv_base.h"
#include "../conf/kv_conf.h"
#include "data_file_tool.h"
#include "data_file_conf.h"



#pragma pack(4)

namespace pingkv
{
typedef int block_id_t;
typedef int row_len_t;
typedef short row_idx_t;
    
class data_file: public kv_base
{
public:
    /******************************************************************************************
     *              struct, typedef
     ******************************************************************************************/
    
    
    struct file_info
    {
        pid_t  curr_pid;
        int  file_id;
        char db_name[16];
        int  block_size;
        int  block_count;
        int  used_block_count;
    };
    
    struct file_head
    {
        file_info info;
        char empy[PINGKV_DBF_BLOCKSIZE - sizeof(file_info)];
    };
    
    struct record_index
    {
        /***
         * low 4-bit flags
         * ... 0   x   x   x 
         *         |   |   |
         *         |   |   |
         *         |   |   +----delete flag
         *         |   |
         *         |   +-----chained row: first string in rowdata is chained rowid
         *         |
         *         +----migrate row: first string in rowdata is real rowid
         */
        char flags;
        short offset;
        int len;
        int next_block_id;
        short next_row_idx;
    };
    

    struct block_head
    {
        long checksum;
        block_id_t block_id;
        int record_count;
    };
    
    struct block_tail
    {
        long checksum;
    };
    
    
    
    /* 
    * +------------------------------+ 
    * | block_head                   |
    * +------------------------------+
    * | record index                 |
    * | record index                 |
    * | ...                          |
    * | ...                          |
    * | free space                   |
    * | ...                          |
    * | ...                          |
    * | data records                 |
    * | data records                 |
    * +------------------------------+
    * | block_tail                   |
    * +------------------------------+
    * 
    */
    struct data_block
    {
        block_head head;
        char data[PINGKV_DBF_BLOCKSIZE - sizeof(block_head) - sizeof(block_tail)]; /* include record indexes */
        block_tail tail;
    };
    
private:
    int _fd;
    string _path_name;
    file_head* _file_head;
    bool _init_ok;
    
    /* 
     * block_id, data_block* 
     */
    std::map<int, data_block*> _cache;
    
    
    
    /******************************************************************************************
     *              file function
     ******************************************************************************************/
    int _extend_file(const int block_count);
    
    /******************************************************************************************
     *              block function
     ******************************************************************************************/
    int _read_file_head();
    int _flush_file_head();
    
    int _format_block(data_block* p, const block_id_t block_id);
    int _read_block(data_block** p, const block_id_t block_id);
    
    int _block_free_bytes(const block_id_t block_id);
    int _block_free_bytes(const data_block* block);
    
//    int _block_checksum(const block_id_t block_id);
    int _block_checksum(data_block* block);
    
    // return free space in block or -1
    int _find_block_can_write(data_block** block);
    
    int _flush_out_block(const block_id_t block_id);
    int _flush_out_block(data_block* block);
    int _flush_out_cache();
    int _flush_block(const block_id_t block_id);
    int _flush_block(data_block* block);
    
    /******************************************************************************************
     *              row function
     ******************************************************************************************/
    int _write_row(short* row_idx, const block_id_t block_id, const char* data, int len);
    int _write_row(short* row_idx, data_block* block, const char* data, int len);
    int _modify_row(const block_id_t block_id, const row_idx_t idx, const char* row, const int len);
    int _delete_row(const block_id_t block_id, const row_idx_t idx);
    int _row_count(const block_id_t block_id);
    int _row_count(const data_block* block);
    int _read_row(char** row, const block_id_t block_id, const row_idx_t idx);
    
    // return row index
    int _row_add_record(data_block* b, int len, bool chained);
    int _row_get_record(record_index** record, const data_block* b, row_idx_t idx);
    int _row_get_pos(char** pos, data_block* b, row_idx_t idx);
    int _row_get_len_in_block(data_block* b, row_idx_t idx);
    
public:
    /******************************************************************************************
     *              constructor, init, destroy
     ******************************************************************************************/
    data_file();
    int create_file(const string& dbname, int file_id, const string& path_name);
    int open_file(const string& path_name);
    int exists(const string& path_name);
    virtual ~data_file();
    
    /******************************************************************************************
     *              user interface
     ******************************************************************************************/
    int block_count() const;
    int used_block_count() const;
    bool init_ok() const;
    
    
    /*
     * return value
     * 0 row deleted
     * n row len
     * -1 error 
     * 
     * !! row need free manully !!
     */
    int read_row(char** row, const block_id_t block_id, const short row_idx);
    
    // write position will fill into rowid
    int write_row(int* block_id, short* row_idx, const char* row, const int len);
    
    int write_row_new_block(int* block_id, const char* row, const int len);
    
    // return 0 or errbi*-1
    int modify_row(const block_id_t block_id, const short row_idx, const char* row, const int len);
    
    int delete_row(const block_id_t block_id, const short row_idx);
    
    int block_free_bytes(const block_id_t block_id);
    
    int empty_block_space();
    
    void dump_block(block_id_t block_id);
    
    int flush_blocks();
};

#define PINGKV_DBF_BLOCK_MIN_FREE (sizeof(pingkv::data_file::record_index))

/* TEST

void fill(char* p, char c, int len)
{
    for(int i=0; i<len; i++) p[i] = c;
}

int main(int argc, char** argv) 
{
    
    pingkv::data_file f;
    
    int re = 0;
    char data[3000];
    char* r = NULL;
    
    block_id_t block_id = 0;
    short row_idx = 0;
    
    system("rm -f /tmp/aaaa");re = f.create_file("testdb", 1, "/tmp/aaaa");
    if(re != 0) goto err;
    
    
    for(int i=0; i<9; i++)
    {
        fill(data, pingkv::tool::itostr(i).at(0), 3000);
        if(f.write_row(&block_id, &row_idx, data, 3000) == -1) goto err;
        if(f.modify_row(block_id, row_idx, "12345", 6) == -1) goto err;
        if(f.delete_row(block_id, row_idx) == -1)             goto err;
        if((re = f.read_row(&r, block_id, row_idx)) == -1)    goto err;
        
        cout << block_id << "_" << row_idx << "  " << (re ? string(r) : "deleted") << endl;
        if(r != NULL) free(r);
        r = NULL;
    }
    if(f.flush_blocks() == -1) goto err;
    
    return 0;
 
    err:
    f.log_err();
    exit(1);  
    
}

 
 */


}
#endif /* DATA_FILE_H */

