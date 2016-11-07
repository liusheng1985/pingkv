/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_server.cc
 * Author: liusheng
 * 
 * Created on 2016年7月12日, 下午2:01
 */

#include "kv_server.h"

namespace pingkv
{
    
kv_server::kv_server(const kv_conf_data& cd)
{
    _listen_sock = -1;
    kv_work_queue* buf = NULL;
    kv_worker* worker = NULL;
    
    _module_name = "KV_SERVER";
    _cd = cd;
    
    _local_file_count = _cd.file_count / _cd.server_count;
    while(_local_file_count * _cd.server_count < _cd.file_count) _local_file_count++;
    _fileno_low = _cd.server_id*_local_file_count;
    if(_cd.server_id == _cd.server_count-1) _local_file_count -= (_local_file_count * _cd.server_count - _cd.file_count);
    
    _res_queue = new kv_work_queue();

    for(int i=0; i<_local_file_count; i++) 
    {
        buf = new kv_work_queue();
        _req_queue.push_back(buf);
        worker = new kv_worker(
            _cd.db_name, 
            _fileno_low + i, 
            data_file_tool::make_file_name(_cd.file_path, _cd.db_name, _fileno_low + i), 
            _cd.create_file,
            buf,
            _res_queue
        );
        _workers.push_back(worker);
    }
}

kv_server::~kv_server() {
    for(std::vector<kv_work_queue*>::iterator it=_req_queue.begin(); it!=_req_queue.end(); it++) delete *it;
    for(std::vector<kv_worker*>::iterator it=_workers.begin(); it!=_workers.end(); it++) delete *it;
    for(std::map<int, kv_work*>::iterator it=_work_bufs.begin(); it!=_work_bufs.end(); it++) 
    {
        close(it->first);
        _pl->unregist(it->first);
        delete it->second;
    }
    delete _res_queue;
    if(_listen_sock > 0) close(_listen_sock);
}

int kv_server::serv()
{
    char e[BUFSIZ];
    boost::thread* thr = NULL;
    if(_do_listen() == -1)
    {
        sprintf(e, "listen network error\n");
        goto err;
    }
    if(_pl.regist_r(_listen_sock, boost::bind(&kv_server::_cb_accept, this, _1, _2), this) == -1)
    {
        sprintf(e, "regist receive callback error\n");
        goto err;
    }
    
    
    _sender = new kv_sender(_res_queue);
    if(_sender == NULL)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        abort();
    }
    _send_thread = new boost::thread(boost::bind(&kv_sender::thread_func, _sender));
    if(_send_thread == NULL)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        abort();
    }
    
    for(int i=0; i<_local_file_count; i++) 
    {
        thr = new boost::thread(boost::bind(&kv_worker::work, _workers[i]));
        if(thr == NULL)
        {
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            abort();
        }
        _threads.push_back(thr);
    }
    _pl.loop();
    return 0;
    
    err:
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
    exit(1);
}

int kv_server::_no_block(int fd)
{
    int fl = 0;
    int r  = 0;
    if((fl = fcntl(fd, F_GETFL, 0)) < 0) goto err;
    if((r  = fcntl(fd, F_SETFL, fl|O_NONBLOCK)) < 0) goto err;
    return 0;

    err:
    return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
}

int kv_server::_do_listen()
{
    int r = 0;
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &servaddr.sin_addr);
    servaddr.sin_port = htons(_cd.port);
    if((_listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    int opt = 1;
    setsockopt(_listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if((r = bind(_listen_sock, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    if(listen(_listen_sock, 1024) == -1) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    return _listen_sock;
}

void kv_server::_cb_accept(int fd, void* arg)
{
    struct sockaddr_in caddr;
    socklen_t addrlen = sizeof(sockaddr_in);
    int cfd;
    char e[BUFSIZ];

    if((cfd = accept(_listen_sock, (struct sockaddr*)&caddr, &addrlen)) < 0)
    {
        //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
//        else
//        {
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            goto err;
//        }
    }

    if(_pl.regist_r(cfd, boost::bind(&kv_server::_cb_read_sock, this, _1, _2), this) == -1)
    {
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "regist receive event error\n");
        goto err;
    }
    
    if(_new_work_buf(cfd) == -1)
    {
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "create new work buffer error\n");
        goto err;
    }
    
    return;
    err:
    if(fd > 0)
    {
        close(cfd);
        _pl->unregist(cfd);
    }
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "accept connect error\n");
    return;
}

int kv_server::_new_work_buf(int fd)
{
    kv_work* w = new_kv_work();
    if(w == NULL) return kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
    _work_bufs[fd] = w;
    return 0;
}

void kv_server::_cb_read_sock(int fd, void* arg)
{
    kv_work* w = _work_bufs[fd];
    int len = 0;
    std::string e = "";
    
    if(len = recv(fd, &w->head, sizeof(kv_work_head), 0) == -1)
    {
        //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "resv error\n");
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        goto err;
    }
    
    if(w->head.field1_len)
    {
        if(w->field1==NULL && (w->field1 = new char[w->head.field1_len])==NULL)
        {
            e.append("alloc error: ");
            goto err;
        }
        if((len = recv(fd, w->field1, w->head.field1_len, 0)) != w->head.field1_len)
        {
            //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
            kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "resv error\n");
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            goto err;
        }
    }
    
    if(w->head.field2_len)
    {
        if(w->field2==NULL && (w->field2 = new char[w->head.field2_len])==NULL)
        {
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            goto err;
        }
        
        if((len = recv(fd, w->field2, w->head.field2_len, 0)) != w->head.field2_len)
        {
            //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
            kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "resv error\n");
            kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
            goto err;
        }
    }
    
        if((w->head.flag & KV_WORK_FLAG_REQ) == KV_WORK_FLAG_REQ)
        {
            long hash = hash_data(w->field1, w->head.field1_len);
            hash = hash%_cd.file_count;
            kv_work_queue* q = _req_queue[hash];
            q->lock_queue();
            q->put_work(fd, w);
            q->signal_waiter();
            q->unlock_queue();
            assert(_new_work_buf(fd) != -1);
        }
        else if((w->head.flag & KV_WORK_FLAG_CMD) == KV_WORK_FLAG_CMD)
        {
            // received a dba command
        }
//    }
//    else goto err;
    
    
    /*
    if(w->head_offset < sizeof(w->head))
    {
        if(w->head_offset == 0) cout << "begin read " << testid << " "  << tool::now_usec() << endl;
        len = recv(fd, (&w->head)+w->head_offset, sizeof(kv_work_head)-w->head_offset, 0);
        if(len == -1)
        {
            //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
            e.append("resv error: ");
            goto err;
        }
        w->head_offset += len;
    }
    else if(w->field1_offset < w->head.field1_len)
    {
        if(w->field1 == NULL) w->field1 = new char[w->head.field1_len];
        len = recv(fd, w->field1+w->field1_offset, w->head.field1_len-w->field1_offset, 0);
        if(len == -1)
        {
            //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
            
            
            sprintf(ce, "%s\n", strerror(errno));
            e.append("resv error: ");
            e.append(ce);
            goto err;
        }
        w->field1_offset += len;
    }
    else if (w->field2_offset < w->head.field2_len)
    {
        if(w->field2 == NULL) w->field2 = new char[w->head.field2_len];
        len = recv(fd, w->field2+w->field2_offset, w->head.field2_len-w->field2_offset, 0);
        if(len == -1)
        {
            //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
            e.append("resv error: ");
            goto err;
        }
        w->field2_offset += len;
    }
    
    if(w->head_offset == sizeof(w->head) && w->field1_offset == w->head.field1_len && w->field2_offset == w->head.field2_len)
    {
        cout << "end read " << testid << " "  << tool::now_usec() << endl;
        if((w->head.flag & KV_WORK_FLAG_REQ) == KV_WORK_FLAG_REQ)
        {
            long hash = hash_data(w->field1, w->head.field1_len);
            hash = hash%_cd.file_count;
            kv_work_queue* q = _req_queue[hash];
            q->lock_queue();
            q->put_work(fd, w);
            q->signal_waiter();
            q->unlock_queue();
            assert(_new_work_buf(fd) != -1);
        }
        else if((w->head.flag & KV_WORK_FLAG_CMD) == KV_WORK_FLAG_CMD)
        {
            // received a dba command
        }
    }
    */
    
    return;
    err:
    kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), "read data from network error\n");
    close(fd);
    _pl->unregist(fd);
    return;
}

/*
void kv_server::_cb_write_sock(int fd, void* arg)
{
    int len = 0;
    std::string e = "";
    kv_work* w = NULL;
    while(1)
    {
        w = _res_bufs[fd];
        if(w == NULL)
        {
            _res_map.lock_map();
            if(_res_map.size() == 0)
            {
                _res_map.unlock_map();
                continue;
            }
            if(_res_map.get_resault(fd, &w) == -1)
            {
                e.append("get resault error\n");
                goto err;
            }
            _res_map.unlock_map();
            if(w == NULL) return;
            _res_bufs[fd] = w;
        }
        
        if(w->head_offset < sizeof(w->head))
        {
            len = send(fd, (&w->head)+w->head_offset, sizeof(kv_work_head)-w->head_offset, 0);
            if(len == -1)
            {
                //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
                e.append("resv error: ");
                goto err;
            }
            w->head_offset += len;
        }
        else if(w->field1_offset < w->head.field1_len)
        {
            len = send(fd, w->field1+w->field1_offset, w->head.field1_len-w->field1_offset, 0);
            if(len == -1)
            {
                //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
                e.append("resv error: ");
                goto err;
            }
            w->field1_offset += len;
        }
        else if (w->field2_offset < w->head.field2_len)
        {
            len = send(fd, w->field2+w->field2_offset, w->head.field2_len-w->field2_offset, 0);
            if(len == -1)
            {
                //if(errno == EAGAIN || errno == EWOULDBLOCK) return;
                e.append("resv error: ");
                goto err;
            }
            w->field2_offset += len;
        }
        
        if(w->head_offset == sizeof(w->head) && w->field1_offset == w->head.field1_len && w->field2_offset == w->head.field2_len)
        {
            _res_bufs.erase(fd);
            delete w;
        }
    }
    
    return;
    err:
    _err.push_back(e);
    close(fd);
    log_err();
    return;
}

*/


}