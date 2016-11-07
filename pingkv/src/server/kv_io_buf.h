/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_io_buf.h
 * Author: liusheng
 *
 * Created on 2016年8月11日, 上午11:39
 */

#ifndef KV_IO_BUF_H
#define KV_IO_BUF_H
#include "../tool/kv_base.h"

namespace pingkv
{
class kv_io_buf: public kv_base
{
public:
    struct block
    {
        char* data;
        short r_offset;
        short w_offset;
    };
    
private:
    int _block_size;
    std::queue<block*> _blocks;
    
    // return block.data
    char* _push_new_block();
    void _del_queue_front();
    
public:
    kv_io_buf();
    kv_io_buf(int block_size);
    virtual ~kv_io_buf();
    
    
    // return data size in buffer
    int size();
    int block_size();
    
    char* r_ptr();
    char* w_ptr();
    
    int r_len();
    int w_len();
    
    /**
     * add offset after read from buffer
     * len = read(fd, buf->w_ptr(), buf->w_len())
     * buf->add_r_offset(len);
     * or
     * len = write(fd, buf->r_ptr(), buf->r_len())
     * buf->add_w_offset(len);
     * 
     * @param offset
     * @return 
     */
    int add_r_offset(int add);
    int add_w_offset(int add);
    
};
}

/* TEST:

void write_file(pingkv::kv_io_buf* buf, pthread_mutex_t* x, pthread_cond_t* c)
{
    int fd = open("/root/test.txt", O_RDWR);
    int len = 0;
    if(fd == -1)
    {
        printf("%s\n", strerror(errno));
        return;
    }
    int i = 0;
    while(1)
    {
        pthread_mutex_lock(x);
        while(buf->size() == 0) pthread_cond_wait(c, x);
        while(buf->size() > 0)
        {
            if((len = write(fd, buf->r_ptr(), buf->r_len())) == -1)
            {
                printf("%s\n", strerror(errno));
                return;
            }
            buf->add_r_offset(len);
            cout << "writefile " << (++i) << " " << len << endl;
        }
        pthread_mutex_unlock(x);
    }
}

void read_file(pingkv::kv_io_buf* buf, pthread_mutex_t* x, pthread_cond_t* c)
{
    int fd = open("/root/SystemErr.log", O_RDONLY);
    int len = 0;
    if(fd == -1)
    {
        printf("%s\n", strerror(errno));
        return;
    }
    int i = 0;
    while(1)
    {
        pthread_mutex_lock(x);
        if((len = read(fd, buf->w_ptr(), buf->w_len())) == -1) return;
        if(len > 0)
        {
            buf->add_w_offset(len);
            cout << "readfile " << (++i) << " " << len << endl;
            pthread_cond_signal(c);
            pthread_mutex_unlock(x);
        }
        else 
        {
            pthread_mutex_unlock(x);
            close(fd);
            return;
        }
    }
}



int main(int argc, char** argv) 
{
    pthread_mutex_t x;
    pthread_cond_t c;
    pingkv::kv_io_buf buf;
    pthread_mutex_init(&x, NULL);
    pthread_cond_init(&c, NULL);
    boost::thread tr(boost::bind(&read_file, &buf, &x, &c));
    boost::thread tw(boost::bind(&write_file, &buf, &x, &c));
    tr.join();
    tw.join();
    pthread_mutex_destroy(&x);
    pthread_cond_destroy(&c);
    
    return 0;
}
 
 */


#endif /* KV_IO_BUF_H */








