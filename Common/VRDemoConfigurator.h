#pragma once

#include <list>
#include <map>
#include <string>

#include "util\l4util.h"
#include "util\L4Singleton.hpp"

class VRDemoConfigurator : public L4Singleton<VRDemoConfigurator>
{
public:
    typedef std::map<std::string, std::string> KeyValueMap;
    typedef std::map<std::string, KeyValueMap> SectionMap;

    bool init(const std::string& configFilePath);
    const SectionMap& getSections() const;
    bool findSection(const std::string& sectionName, const KeyValueMap **map) const;

    template<typename T>
    bool getValue(const std::string& sectionName, const std::string& key, T* typeValue) {
        bool result = false;
        const KeyValueMap *map = nullptr;

        if (findSection(sectionName, &map)) {
            auto const it = map->find(key);
            if (it != map->end()) {
                if (typeValue) {
                    result = valueToType(it->second, typeValue);
                }
                else {
                    result = true;
                }
            }
        }
        return result;
    }
    template<typename T>
    bool valueToType(const std::string& value, T* typeValue) { return false; }
    bool VRDemoConfigurator::valueToType(const std::string& value, bool* typeValue);
    template<typename T>
    bool saveValue(const std::string& sectionName, const std::string& key, const T& typeValue) {
        return saveValue(sectionName, key, typeToValue(typeValue));
    }
    template<typename T>
    std::string typeToValue(const T& typeValue) { return "UNDEFINED"; }
    std::string typeToValue(const bool& typeValue);
    bool saveValue(const std::string& sectionName, const std::string& key, const std::string& value);
    static const std::string FILE_SETTINGS;
private:
    inline bool isInited() const { return 0 < m_configFilePath.size(); };
    void parseSection(const std::string& configFilePath, const std::string& sectionName);
    SectionMap m_sectionMap;
    std::string m_configFilePath;
};