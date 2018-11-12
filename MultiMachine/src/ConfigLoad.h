#ifndef CONFIGLOAD_H
#define CONFIGLOAD_H
#include <memory>           //std::shared_ptr<>
#include <unordered_map>    //unordered_map
#include <string>           //string


namespace nqueens
{
 /**
 * 功能：加载配置文件
 * 定义为singleton类，反正从一开始就得加载，直接用饿汉式
 */
class ConfigLoad
{
public:
    ~ConfigLoad();
    static const std::shared_ptr<ConfigLoad> getIns(){return pConfigLoad_;}
    const std::string& getValue(const std::string& key) const
    {
        auto iter = key2Value_.find(key);
        if (iter != key2Value_.end())
            return iter->second;
        throw std::runtime_error("No Such Value"); //TODO：换个友好的方式
    }
    void trim(std::string &str);

    ConfigLoad(const ConfigLoad&) = delete;
    ConfigLoad& operator=(const ConfigLoad&) = delete;

private:
    ConfigLoad(const std::string &filename);

    static std::shared_ptr<ConfigLoad> pConfigLoad_;
    std::unordered_map<std::string, std::string> key2Value_;
};
}
#endif 