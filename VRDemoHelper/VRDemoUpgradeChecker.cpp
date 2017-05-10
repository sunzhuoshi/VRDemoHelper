#include "stdafx.h"
#include "VRDemoUpgradeChecker.h"

#include <fstream>

#include <Shellapi.h>
#include "json\json.h"
#include "log4cplus\log4cplus.h"
#include "curl\curl.h"

const std::string VRDemoUpgradeChecker::DATA_FILE_NAME = "VRDemoHelper.dat";
const std::string VRDemoUpgradeChecker::PROPERTY_LATEST_VERSION_URL = "latestVersionURL";
const std::string VRDemoUpgradeChecker::PROPERTY_VERSION = "version";
const std::string VRDemoUpgradeChecker::PROPERTY_DOWNLOAD_PAGE_URL = "downloadPageURL";

bool VRDemoUpgradeChecker::init()
{
    Json::Reader reader;
    std::ifstream file(DATA_FILE_NAME);
    Json::Value root;
    log4cplus::Logger logger = log4cplus::Logger::getRoot();

    if (file.good()) {
        if (reader.parse(file, root, false)) {
            bool ret = m_localVersion.fromString(root[PROPERTY_VERSION].asString());
            if (!ret) {
                LOG4CPLUS_ERROR(logger, "Failed to get version info in file: " << DATA_FILE_NAME);
            }
            m_latestVersionURL = root[PROPERTY_LATEST_VERSION_URL].asString();
            return ret;
        }
        else {
            LOG4CPLUS_ERROR(logger, "Failed to parse data file: " << DATA_FILE_NAME << ", error: " << reader.getFormattedErrorMessages());
        }
    }
    else {
        LOG4CPLUS_ERROR(logger, "Failed to open data file: " + DATA_FILE_NAME);
    }
    return false;
}

void VRDemoUpgradeChecker::asynCheckUpgrade()
{
    m_thread = std::thread(&VRDemoUpgradeChecker::run, this);
}

void VRDemoUpgradeChecker::run()
{
    CURL *curl = curl_easy_init();
    CURLcode ret;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, m_latestVersionURL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, VRDemoUpgradeChecker::receiveCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        ret = curl_easy_perform(curl);
        if (CURLE_OK != ret) {
            LOG4CPLUS_INFO(log4cplus::Logger::getRoot(), curl_easy_strerror(ret));
        }
        curl_easy_cleanup(curl);
    }
}

VRDemoUpgradeChecker::VRDemoUpgradeChecker()
{
}


VRDemoUpgradeChecker::~VRDemoUpgradeChecker()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

size_t VRDemoUpgradeChecker::receiveCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realSize = size * nmemb;
    VRDemoUpgradeChecker *checker = reinterpret_cast<VRDemoUpgradeChecker*>(userp);

    char *buf = new (std::nothrow)char[realSize + 1];
    if (buf) {
        memcpy(buf, contents, realSize);
        buf[realSize] = 0;
        checker->processLatestVersionInfo(buf);
        delete[] buf;
        buf = nullptr;
    }
    else {
        realSize = 0;
    }
    return realSize;
}


void VRDemoUpgradeChecker::processLatestVersionInfo(const std::string& jsonString)
{
    Json::Reader reader;
    Json::Value root;
    log4cplus::Logger logger = log4cplus::Logger::getRoot();

    if (reader.parse(jsonString, root)) {
        std::string downloadPageURL = root[PROPERTY_DOWNLOAD_PAGE_URL].asString();
        Version version;
        if (version.fromString(root[PROPERTY_VERSION].asString())) {
            if (0 < version.compare(m_localVersion)) {
                if (IDYES == MessageBoxA(NULL, "VRDemoHelper upgrade available, do you want to check the download page?", "Promot", MB_YESNO)) {
                    ShellExecuteA(NULL, "open", downloadPageURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
                }
            }
        }
    }
}

