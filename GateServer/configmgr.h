#ifndef CONFIGMGR_H
#define CONFIGMGR_H

#include "const.h"
#include "singleton.h"

/******************************************************************************
 * @file       configmgr.h
 * @brief      读取和管理配置的类
 *
 * @author     lueying
 * @date       2025/12/18
 * @history
 *****************************************************************************/

// 管理配置文件的key和value
struct SectionInfo {
    SectionInfo() {}
    ~SectionInfo() {
        _section_datas.clear();
    }

    SectionInfo(const SectionInfo& src) {
        _section_datas = src._section_datas;
    }

    SectionInfo& operator = (const SectionInfo& src) {
        if (&src == this) {
            return *this;
        }

        this->_section_datas = src._section_datas;
    }

    std::map<std::string, std::string> _section_datas;
    std::string  operator[](const std::string& key) {
        if (_section_datas.find(key) == _section_datas.end()) {
            return "";
        }
        // 这里可以添加一些边界检查  
        return _section_datas[key];
    }
};

class ConfigMgr : public Singleton<ConfigMgr>
{
public:
    ConfigMgr(const ConfigMgr& src) = delete;
    ConfigMgr& operator=(const ConfigMgr& src) = delete;
    ~ConfigMgr() {
        config_map_.clear();
    }
    SectionInfo operator[](const std::string& section) {
        if (config_map_.find(section) == config_map_.end()) {
            return SectionInfo();
        }
        return config_map_[section];
    }

    // 返回一个管理配置的实例
    static ConfigMgr& getInst();

private:
    ConfigMgr();

    // 存储section和key-value对的map  
    std::map<std::string, SectionInfo> config_map_;
};

#endif // CONFIGMGR_H