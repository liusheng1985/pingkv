/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_conf.h
 * Author: liusheng
 *
 * Created on 2016年7月12日, 下午2:00
 */

#ifndef KV_CONF_H
#define KV_CONF_H

#include "../tool/kv_base.h"

namespace pingkv
{
struct kv_conf_data
{
    int server_id;
    char db_name[128];
    char ip[20];
    int port;
    bool create_file;
    char file_path[256];
    int server_count;
    int file_count;
    int lru_cache_size;
    int lru_max_elem_size;
    kv_conf_data();
    ~kv_conf_data();
    kv_conf_data(const kv_conf_data& o);
    kv_conf_data& operator=(const kv_conf_data& o);
    void dump_data();
};

class kv_conf: public kv_base
{
public:
    
private:
    kv_conf();
    virtual ~kv_conf();
    kv_conf(const kv_conf& o);
    static kv_conf* _one;
    bool _read_ok;
    
    enum conf_type {CONF_TYPE_GLOBAL=1, CONF_TYPE_SERVER=2, CONF_TYPE_LOCAL=3};
    int _local_server_id;
    
    std::map<int, kv_conf_data> _server_conf;
    kv_conf_data _global_conf;
    
    int _load_conf (FILE* f, int conf_type);
    std::pair<std::string, std::string> _get_param_val(const std::string& str);
    std::pair<int, std::string> _split_servid_param(const std::string& param);
    
    bool _is_num(const std::string& s);
    int _str_to_bytes(const std::string& s);
    
    int _set_server_param(const std::string& p, const std::string& v);
    int _set_server_param(kv_conf_data* conf, const std::string& p, const std::string& v);
    
    int _set_global_param(const std::string& p, const std::string& v);
    int _set_local_param(const std::string& p, const std::string& v);
    
public:
    static kv_conf& single();
    int read_conf(const std::string& file_name);
    bool is_read_ok();
    void dump_conf();
    
    int get_server_count() const;
    int get_local_server_id() const;
    kv_conf_data get_server_conf(int server_id) const;
};












}
#endif /* KV_CONF_H */

