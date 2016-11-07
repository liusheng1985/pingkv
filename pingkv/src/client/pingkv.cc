/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <sys/time.h>
#include <string>
#include <vector>
#include <string>
#include "string.h"
#include "stdlib.h"
#include "kv_client.h"
#include "../tool/tool.h"

using namespace std;

int main(int argc, char** argv)
{
    int re = 0;
    
    string file_name = "/opt/dev/lskv/testdb.conf";
    pingkv::kv_conf::single().read_conf(file_name);
    if(!pingkv::kv_conf::single().is_read_ok()) abort();
    int local_sid = pingkv::kv_conf::single().get_local_server_id();
    pingkv::kv_conf_data cd = pingkv::kv_conf::single().get_server_conf(local_sid);
    
    vector<pair<string,int> > ss;
    ss.push_back(pair<string, int>(cd.ip, cd.port));
    pingkv::kv_client c(ss);
    c.init();
    
    
    
    char k[20];
    char v[20];
    memset(k, '\0', 20);
    memset(v, '\0', 20);
    int cnt = 0;
//    for(int i=0; i<1000000; i++)
//    {
//        sprintf(k, "%ld", i);
//        sprintf(v, "%ld", i*100);
//        re = c.set(k, strlen(k), v, strlen(v));
//        if(re == -1) abort();
//        cnt++;
//        if(cnt%1000 == 0) cout << cnt << endl;
//    }
    
    
    
    char* vv = NULL;
    int vv_len = 0;
    cout << "begin " << pingkv::tool::now_microsec() << endl;
    for(int i=1; i<10000; i++)
    {
        sprintf(k, "%d", i);
        re = c.get(&vv, &vv_len, k, strlen(k));
        if(re == -1) abort();
//        cout << vv << endl;
        if(vv)
        {
            delete vv;
            vv = NULL;
        }
        cnt++;
        if(cnt%100 == 0) 
            cout << cnt << " " << pingkv::tool::now_microsec() << endl;
        delete vv;
    }
    
}
