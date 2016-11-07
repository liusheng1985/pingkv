/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   kv_conf.cc
 * Author: liusheng
 * 
 * Created on 2016年7月12日, 下午2:00
 */

#include "kv_conf.h"

namespace pingkv
{
kv_conf* kv_conf::_one = NULL;

kv_conf::kv_conf(): _read_ok(false), _local_server_id(-1) {_module_name = "CONFIG";}
kv_conf::kv_conf(const kv_conf& o){}
kv_conf::~kv_conf(){}

kv_conf& kv_conf::single() {if(_one == NULL) _one = new kv_conf(); return *_one;}
bool kv_conf::is_read_ok() {return _read_ok;}


int kv_conf::read_conf(const std::string& file_name)
{
    int ret = 0;
    FILE* f = NULL;
    char e[BUFSIZ];
    if((f = fopen(file_name.data(), "r")) == NULL)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        ret = -1;
        goto end;
    }
    
    if(_load_conf(f, CONF_TYPE_GLOBAL) == -1)
    {
        sprintf(e, "load global config error: file %s block %d row %d\n", file_name.c_str());
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
        ret = -1;
        goto end;
    }
    
    
    if(_load_conf(f, CONF_TYPE_SERVER) == -1)
    {
        sprintf(e, "load server config error: file %s block %d row %d\n", file_name.c_str());
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
        ret = -1;
        goto end;
    }
    
    if(_load_conf(f, CONF_TYPE_LOCAL) == -1)
    {
        sprintf(e, "load local config error: file %s block %d row %d\n", file_name.c_str());
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
        ret = -1;
        goto end;
    }
    
    end:
    _read_ok = (ret == 0);
    if(f != NULL) {fclose(f); f = NULL;}
    return ret;
}

int kv_conf::_str_to_bytes(const std::string& s)
{
    if(s.size() == 0) return -1;
    if(_is_num(s)) return atoi(s.c_str());
    
    const char unit = s[s.size()-1];
    std::string num = s.substr(0, s.size()-1);
    if(!_is_num(num)) return -1;
    if(unit == 'k' || unit == 'K') return atoi(num.c_str()) * 1024;
    if(unit == 'm' || unit == 'M') return atoi(num.c_str()) * 1024 * 1024;
    if(unit == 'g' || unit == 'G') return atoi(num.c_str()) * 1024 * 1024 * 1024;
    return -1;
}

std::pair<std::string, std::string> kv_conf::_get_param_val(const std::string& str)
{
    std::pair<std::string, std::string> ret("","");
    
    boost::regex re("\\s*(\\S+)\\s*=\\s*(\\S+)\\s*");
    boost::cmatch cm;
    if(boost::regex_match(str.c_str(), cm, re))
    {
        if(cm.size() != 3) return ret;
        ret.first = std::string(cm[1]);
        ret.second = std::string(cm[2]);
    }
    return ret;
}

std::pair<int, std::string> kv_conf::_split_servid_param(const std::string& param)
{
    // format: server.N.param_name
    std::pair<int, std::string> ret(-1,"");
    boost::regex re("(\\d+)\\.(\\S+)");
    boost::cmatch cm;
    if(boost::regex_match(param.c_str(), cm, re))
    {
        if(cm.size() != 3) return ret;
        ret.first = atoi(std::string(cm[1]).data());
        ret.second = std::string(cm[2]);
    }
    return ret;
}

bool kv_conf::_is_num(const std::string& s)
{
    if(s.size() == 0) return false;
    for(int i=0; i<s.size(); i++) if(s[i] > '9' || s[i] < '0') return false;
    return true;
}

int kv_conf::_set_local_param(const std::string& p, const std::string& v)
{
    if(p.size() == 0 || v.size() == 0) return -1;
    if(p.compare("server_id") == 0)
    {
        if(!_is_num(v)) return -1;
        _local_server_id = atoi(v.c_str());
        if(_local_server_id < 0) return -1;
    }
    return 0;
}

int kv_conf::_set_global_param(const std::string& p, const std::string& v)
{
    if(p.size()==0 || v.size()==0) return -1;
    return _set_server_param(&_global_conf, p, v);
}

int kv_conf::_set_server_param(const std::string& p, const std::string& v)
{
    if(p.size()==0 || v.size()==0) return -1;
    std::pair<int, std::string> id_param = _split_servid_param(p);
    int id = id_param.first;
    if(id == -1 || id_param.second.size() == 0) return -1;
    if(_server_conf.find(id) == _server_conf.end()) _server_conf[id] = _global_conf;
    if(_set_server_param(&(_server_conf[id]), id_param.second, v) == -1) return -1;
    return 0;
}


int kv_conf::_set_server_param(kv_conf_data* conf, const std::string& p, const std::string& v)
{
    bool isnum = _is_num(v);
    int i = atoi(v.c_str());
    int len = v.size();
    if(p.compare("server_id") == 0)
    {
        if(!isnum || i < 0) return -1;
        conf->server_id = i;
    }
    else if(p.compare("db_name") == 0)
    {
        if(len > 127 || len == 0) return -1;
        strcpy(conf->db_name, v.c_str());
    }
    else if(p.compare("file_path") == 0)
    {
        if(len > 127 || len == 0) return -1;
        strcpy(conf->file_path, v.c_str());
    }
    else if(p.compare("ip") == 0)
    {
        if(len > 15 || len < 7) return -1;
        strcpy(conf->ip, v.c_str());
    }
    else if(p.compare("port") == 0)
    {
        if(!isnum || i < 0) return -1;
        conf->port = i;
    }
    else if(p.compare("create_file") == 0)
    {
        if(v.compare("yes") == 0 || v.compare("YES") == 0 || v.compare("Yes") == 0) conf->create_file = true;
        else if(_is_num(v)) if(atoi(v.c_str()) > 0) conf->create_file = true;
        else conf->create_file = false;
    }
    else if(p.compare("server_count") == 0)
    {
        if(!isnum || i < 1) return -1;
        conf->server_count = i;
    }
    else if(p.compare("file_count") == 0)
    {
        if(!isnum || i < 1) return -1;
        conf->file_count = i;
    }
    else if(p.compare("lru_cache_size") == 0)
    {
        i = _str_to_bytes(v);
        if(i < 0) return -1;
        conf->lru_cache_size = i;
    }
    else if(p.compare("lru_max_elem_size") == 0)
    {
        i = _str_to_bytes(v);
        if(i < 0) return -1;
        conf->lru_max_elem_size = i;
    }
    else return -1;
    
    return 0;
}

int kv_conf::_load_conf(FILE* f, int conf_type)
{
    if(f == NULL) return -1;
    if(fseek(f, 0, SEEK_SET) != 0)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    
    char buf[BUFSIZ];
    std::string line;
    
    char line_head[10];
    switch(conf_type)
    {
        case CONF_TYPE_SERVER:
            strcpy(line_head, "server.");
            break;
        case CONF_TYPE_GLOBAL: 
            strcpy(line_head, "all.");
            break;
        case CONF_TYPE_LOCAL:
            strcpy(line_head, "local.");
            break;
        default:
            return -1;
    };

    while(fgets(buf, BUFSIZ, f) != NULL)
    {
        char e[BUFSIZ];
        buf[strlen(buf)-1] = '\0';
        line = buf;
        if(line.find(line_head) != 0) continue;
        
        std::pair<std::string, std::string> pv = _get_param_val(line);
        pv.first = pv.first.substr(strlen(line_head));
        if(pv.first.size() == 0 || pv.second.size() == 0)
        {
            sprintf(e, "syntax error in line '%s'\n", line.c_str());
            kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
            return -1;
        }

        if(conf_type == CONF_TYPE_SERVER)
        {
            if(_set_server_param(pv.first, pv.second) == -1)
            {
                sprintf(e, "syntax error in line '%s'\n", line.c_str());
                kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
                return -1;
            }
        }
        else if (conf_type == CONF_TYPE_GLOBAL)
        {
            if(_set_global_param(pv.first, pv.second) == -1)
            {
                sprintf(e, "syntax error in line '%s'\n", line.c_str());
                kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
                return -1;
            }
        }
        else if (conf_type == CONF_TYPE_LOCAL)
        {
            if(_set_local_param(pv.first, pv.second) == -1)
            {
                sprintf(e, "syntax error in line '%s'\n", line.c_str());
                kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
                return -1;
            }
        }
    }
    if(!feof(f))
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    
    if(fseek(f, 0, SEEK_SET) != 0)
    {
        kv_logger::get()->oserrlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), errno);
        return -1;
    }
    
    return 0;
}



int kv_conf::get_server_count() const
{
    return _server_conf.size();
}
int kv_conf::get_local_server_id() const
{
    return _local_server_id;
}
kv_conf_data kv_conf::get_server_conf(int server_id) const
{
    char e[BUFSIZ];
    std::map<int, kv_conf_data>::const_iterator it = _server_conf.find(server_id);
    if(it == _server_conf.end())
    {
        sprintf(e, "bad server id:%d\n", server_id);
        kv_logger::get()->errlog_fmt(__FILE__, __func__, __LINE__, _module_name.c_str(), e);
        exit(1);
    }
    return it->second;
}






void kv_conf::dump_conf()
{
    if(!_read_ok)
    {
        std::cout << "config not loaded yet." << std::endl;
        return;
    }
    
    std::cout << "config dump begin ..." << std::endl;
    std::cout << "local server id: " << _local_server_id << std::endl;
    
    std::cout << "dump global config ..." << std::endl;
    _global_conf.dump_data();
    std::cout << std::endl;
    std::cout << "dump server config ..." << std::endl;
    for(std::map<int, kv_conf_data>::iterator it=_server_conf.begin(); it!=_server_conf.end(); it++)
    {
        it->second.dump_data();
        std::cout << std::endl;
    }
    std::cout << "config dump end." << std::endl;
    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////


kv_conf_data::kv_conf_data():
server_id         (0),
port              (7000),
create_file       (true),
server_count      (1),
file_count        (1),
lru_cache_size    (1024*1024*10),
lru_max_elem_size (1024*256)
{
    db_name   [0] = '\0';
    ip        [0] = '\0';
    file_path [0] = '\0';
}

kv_conf_data::~kv_conf_data(){}

kv_conf_data::kv_conf_data(const kv_conf_data& o)
{
    server_id         = o.server_id        ;
    port              = o.port             ;
    create_file       = o.create_file      ;
    server_count      = o.server_count     ;
    file_count        = o.file_count       ;
    lru_cache_size    = o.lru_cache_size   ;
    lru_max_elem_size = o.lru_max_elem_size;
    strcpy(ip       , o.ip       );
    strcpy(db_name  , o.db_name  );
    strcpy(file_path, o.file_path);
}

kv_conf_data& kv_conf_data::operator=(const kv_conf_data& o)
{
    server_id         = o.server_id        ;
    port              = o.port             ;
    create_file       = o.create_file      ;
    server_count      = o.server_count     ;
    file_count        = o.file_count       ;
    lru_cache_size    = o.lru_cache_size   ;
    lru_max_elem_size = o.lru_max_elem_size;
    strcpy(ip       , o.ip       );
    strcpy(db_name  , o.db_name  );
    strcpy(file_path, o.file_path);
    return *this;
}

void kv_conf_data::dump_data()
{
    std::cout << "server_id         : " << server_id << std::endl;
    std::cout << "db_name           : " << db_name << std::endl;
    std::cout << "ip                : " << ip << std::endl;
    std::cout << "port              : " << port << std::endl;
    std::cout << "server_count      : " << server_count << std::endl;
    std::cout << "file_count        : " << file_count << std::endl;
    std::cout << "lru_cache_size    : " << lru_cache_size << std::endl;
    std::cout << "lru_max_elem_size : " << lru_max_elem_size << std::endl;
    std::cout << "create_file       : " << create_file << std::endl;
    std::cout << "file_path         : " << file_path << std::endl;
}

}












