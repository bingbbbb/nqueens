#include <iostream>
#include <fstream>
#include "ConfigLoad.h"

using namespace nqueens;
using std::string;

//饿汉式定义ConfigLoad对象(main之前)
std::shared_ptr<ConfigLoad> ConfigLoad::pConfigLoad_(new ConfigLoad("./conf/nqueens.conf"));
/**
 * 上面的路径("./conf/nqueens.conf")挺有意思的，如果在MultiMachine目录下
 * 运行编译连接命令(sh ./build.sh)，那么前缀应该是./，如果是单独运行./build.sh
 * 文件里面的各条命令，即cd到build目录下再cmake、make，那么这里需要改成../，这里
 * 是用相对于cmake命令时所在的路径，而不是ConfigLoad.cpp所在的路径
 */
ConfigLoad::~ConfigLoad()
{
    //std::cout << "ConfigLoad destruct" << std::endl;
}

/**
 * 加载配置文件
 * @param string&
 */
ConfigLoad::ConfigLoad(const string &filename)
{
    std::ifstream iConf(filename);
    if (!iConf.good())
    {
        //加载配置文件时，日志类还没初始化（日志文件名就在配置文件里）
        std::cerr << filename << " config file open failed" << std::endl;
        exit(-1);
    }
    string stBuf;
    while (getline(iConf, stBuf))
    {
        if (stBuf.empty())
            continue;
        if (stBuf.find("//") == 0 || stBuf.find("#") == 0) //注释行
            continue;
        size_t pos = stBuf.find("=");
        if (pos == string::npos)
            continue;
        string key = stBuf.substr(0, pos);    //取等号前
        string value = stBuf.substr(pos + 1); //取等号后
        trim(value);
        trim(key);
        key2Value_[key] = value;
    }
    iConf.close();
}

/**
 * 去除空格
 * @param string&
 */
void ConfigLoad::trim(string &str)
{
    if (str.empty())
        return;
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
}