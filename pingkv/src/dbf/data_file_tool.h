/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   data_file_tool.h
 * Author: liusheng
 *
 * Created on 2016年6月6日, 上午10:07
 */

#ifndef DATA_FILE_TOOL_H
#define DATA_FILE_TOOL_H
#include <string>
namespace pingkv
{


#define PINGKV_DBF_ROW_FLAG_BITS (4)
#define PINGKV_DBF_ROW_DELETED(flags)  ((flags) & 1)
#define PINGKV_DBF_ROW_CHAINED(flags)  (((flags) & 2)  >> 1)
#define PINGKV_DBF_ROW_MIGRATED(flags) (((flags) & 4 ) >> 2)

#define PINGKV_DBF_ROW_SET_DELETED(flags)  ((flags) | 1)
#define PINGKV_DBF_ROW_SET_CHAINED(flags)  ((flags) | 2)
#define PINGKV_DBF_ROW_SET_MIGRATED(flags) ((flags) | 4)


    
class data_file_tool
{
public:
    static std::string int_to_rowidstr(int l)
    {
        if(l == 0) return std::string("0");
            char num[62] = {'0','1','2','3','4','5','6','7','8','9',
        'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
        };

        char rowid[20];
        int i = 19;
        rowid[i] = '\0';
        int tmp = 0;
        bool sign = l < 0;
        if(sign) l *= -1;
        while(l > 0)
        {
            i--;
            tmp = l/62;
            rowid[i] = num[l-tmp*62];
            l = tmp;
        }
        if(sign) rowid[--i] = '-';
        return std::string(rowid + i);
    }

    
    static int rowidstr_to_int(const std::string& rowidstr)
    {
        int num[123];
        num['a'] = 10;   num['A'] = 36;   num['k'] = 20;   num['K'] = 46;
        num['b'] = 11;   num['B'] = 37;   num['l'] = 21;   num['L'] = 47;
        num['c'] = 12;   num['C'] = 38;   num['m'] = 22;   num['M'] = 48;
        num['d'] = 13;   num['D'] = 39;   num['n'] = 23;   num['N'] = 49;
        num['e'] = 14;   num['E'] = 40;   num['o'] = 24;   num['O'] = 50;
        num['f'] = 15;   num['F'] = 41;   num['p'] = 25;   num['P'] = 51;
        num['g'] = 16;   num['G'] = 42;   num['q'] = 26;   num['Q'] = 52;
        num['h'] = 17;   num['H'] = 43;   num['r'] = 27;   num['R'] = 53;
        num['i'] = 18;   num['I'] = 44;   num['s'] = 28;   num['S'] = 54;
        num['j'] = 19;   num['J'] = 45;   num['t'] = 29;   num['T'] = 55;

        num['0'] = 0;    num['u'] = 30;   num['U'] = 56;    
        num['1'] = 1;    num['v'] = 31;   num['V'] = 57;
        num['2'] = 2;    num['w'] = 32;   num['W'] = 58;
        num['3'] = 3;    num['x'] = 33;   num['X'] = 59;
        num['4'] = 4;    num['y'] = 34;   num['Y'] = 60;
        num['5'] = 5;    num['z'] = 35;   num['Z'] = 61;
        num['6'] = 6;         
        num['7'] = 7;         
        num['8'] = 8;         
        num['9'] = 9;          

        const char* str = rowidstr.data();
        int len = rowidstr.size();
        int ret = 0;
        for(int i=0; i<len; i++)
        {
            ret *= 62;
            ret += num[str[i]];
        } 
        return ret;
    }

    static int count_char(const std::string& str, char c)
    {
        const char* cs = str.c_str();
        int re = 0;
        for(int i=0; i<str.size(); i++) if(*(cs+i) == c) ++re;
        return re;
    }
    
    static std::string get_sub_str(std::string str, const std::string& split, int idx)
    {
        int from = 0;
        int to = str.find("_");
        if(idx == 1) return str.substr(from, to-from);
        else
        {
            from = -1;
            for(int i=0; i<idx-1; i++) from = str.find("_", from+1);
            if((to = str.find("_", from+1)) > str.size()) to = str.size();
            return str.substr(from+1, to-from-1);
        }
    }
    
    // row idx from 1
    static std::string make_rowid(const int block_id, const int row_idx)
    {
        std::string re = int_to_rowidstr(block_id);
        re.append("_").append(int_to_rowidstr(row_idx));
        return re;
    }

    static std::string make_rowid(const int file_id, const int block_id, const int row_idx)
    {
        std::string re = int_to_rowidstr(file_id);
        re.append("_").append(int_to_rowidstr(block_id)).append("_").append(int_to_rowidstr(row_idx));
        return re;
    }
    
    static int rowid_to_file_id(const std::string& rowid)
    {
        if(count_char(rowid, '_') != 3) return -1;
        std::string id = get_sub_str(rowid, "_", 1);
        return rowidstr_to_int(id);
    }

    static int rowid_to_block_id(const std::string& rowid)
    {
        if(count_char(rowid, '_') == 2) return rowidstr_to_int(get_sub_str(rowid, "_", 2));
        else if (count_char(rowid, '_') == 1) return rowidstr_to_int(get_sub_str(rowid, "_", 1));
        else return -1;
    }
    
    static int rowid_to_row_idx(const std::string& rowid)
    {
        if(count_char(rowid, '_') == 2) return rowidstr_to_int(get_sub_str(rowid, "_", 3));
        else if (count_char(rowid, '_') == 1) return rowidstr_to_int(get_sub_str(rowid, "_", 2));
        else return -1;
    }
    
    static std::string make_file_name(const std::string& base_dir, const std::string& db_name, int file_id)
    {
        char i[16];
        sprintf(i, "%d", file_id);
        std::string ret = base_dir;
        if(ret.size() > 0 && ret.at(ret.size()-1) != '/') ret.append("/");
        ret.append(db_name);
        ret.append("_");
        ret.append(i);
        ret.append(".dbf");
        return ret;
    }
    
    
};

}

#endif /* DATA_FILE_TOOL_H */

