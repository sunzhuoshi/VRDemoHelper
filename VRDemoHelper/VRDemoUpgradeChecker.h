#pragma once
#include "util\L4Singleton.hpp"

#include "util\l4util.h"

#include <regex>
#include <thread>

class VRDemoUpgradeChecker : public L4Singleton<VRDemoUpgradeChecker> 
{
public:
    union Version {
        struct {
            int v1;
            int v2;
            int v3;
            int v4;
        };
        int vs[4];
        bool fromString(const std::string& versionString) {
            if (std::regex_match(versionString.c_str(), std::regex("\\d+\\.\\d+\\.\\d+\\.\\d+"))) {
                std::string tmp = versionString;
                std::regex re("\\d+");
                std::smatch sm;
                int *p = reinterpret_cast<int *>(this);
                int i = 0;
             
                while (std::regex_search(tmp, sm, re)) {
                    p[i++] = atoi(sm.str().c_str());
                    tmp = sm.suffix();
                }
                return true;
            }
            return false;
        }
        std::string toString() {
            std::ostringstream str;
            str << v1 << '.' << v2 << "." << v3 << "." << v4;
            return str.str();
        }
        int compare(const Version& other) {
            int result = 0;
            for (int i = 0; i < 4; ++i) {
                if (vs[i] > other.vs[i]) {
                    result = 1;
                    break;
                }
                else if (vs[i] < other.vs[i]) {
                    result = -1;
                    break;
                }
            }
            return result;
        }
    };
    bool init();
    void asynCheckUpgrade();
    void run();

    const Version& getLocalVersion() {
        return m_localVersion;
    }
    VRDemoUpgradeChecker();
    ~VRDemoUpgradeChecker();
private:
    static size_t receiveCallback(void *contents, size_t size, size_t nmemb, void *userp);
    void processLatestVersionInfo(const std::string& jsonString);
    std::thread m_thread;
    Version m_localVersion;
    std::string m_latestVersionURL;
    static const std::string DATA_FILE_NAME;
    static const std::string PROPERTY_LATEST_VERSION_URL;
    static const std::string PROPERTY_VERSION;
    static const std::string PROPERTY_DOWNLOAD_PAGE_URL;
};

