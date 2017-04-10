#include "stdafx.h"

#include "VRDemoConfigurator.h"

#include <sstream>
#include "log4cplus\log4cplus.h"

const std::string VRDemoConfigurator::FILE_SETTINGS = "settings.ini";

bool VRDemoConfigurator::init(const std::string& configFilePath)
{
    bool result = false;
    char buf[1024];
    DWORD p = 0, ret = GetPrivateProfileSectionNamesA(buf, sizeof(buf), configFilePath.c_str());

    if (0 < ret) {
        std::ostringstream sectionName;
        while (p < ret) {
            if ('\0' != buf[p]) {
                sectionName << buf[p];
            }
            else {
                parseSection(configFilePath, sectionName.str());
                sectionName.str("");
                sectionName.clear();
            }
            p++;
        }
        m_configFilePath = configFilePath;
        result = true;
    }
    return result;
}


void VRDemoConfigurator::parseSection(const std::string& configFilePath, const std::string& sectionName)
{
    std::string sectionKey = l4util::toUpper(sectionName);
    if (!m_sectionMap.count(sectionKey)) {
        char buf[1024] = "";
        DWORD p = 0, ret = GetPrivateProfileSectionA(sectionName.c_str(), buf, sizeof(buf), configFilePath.c_str());
        KeyValueMap properties;
        std::ostringstream line;
        bool commented = false;

        while (p < ret) {
            switch (buf[p]) {
            case '\0':      // end of line
            {
                l4util::StringPair keyValue;
                if (!l4util::trim(line.str()).empty()) {
                    if (l4util::parseProperty(line.str(), keyValue)) {
                        std::string originalKey = keyValue.first;
                        l4util::toUpper(keyValue.first);
                        if (!properties.count(keyValue.first)) {
                            properties.insert(keyValue);
                        }
                    }
                }
                line.str("");
                line.clear();
                commented = false;
            }
                break;
            case '#':       // comment out from here
                commented = true;
                break;
            default:
                if (!commented) {
                    line << buf[p];
                }
                break;
            }
            p++;
        }
        if (!properties.empty()) {
            m_sectionMap[sectionKey] = properties;
        }
    }
}
