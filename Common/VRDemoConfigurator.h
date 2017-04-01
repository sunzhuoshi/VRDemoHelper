#pragma once

#include <list>
#include <map>
#include <string>

#include "log4cplus\log4cplus.h"
#include "util\l4util.h"

class VRDemoConfigurator
{
public:
    typedef std::map<std::string, std::string> KeyValueMap;
    typedef std::map<std::string, KeyValueMap> SectionMap;

    static VRDemoConfigurator& VRDemoConfigurator::getInstance() {
        static VRDemoConfigurator instance;
        return instance;
    }
    bool init(const std::string& configFilePath, const std::string& loggerName);
    const SectionMap& getSections() const {
        return m_sectionMap;
    }
    static const std::string FILE_SETTINGS;
private:
    void parseSection(const std::string& configFilePath, const std::string& sectionName);
    SectionMap m_sectionMap;
    std::string m_configFilePath;
    log4cplus::Logger m_logger;
};