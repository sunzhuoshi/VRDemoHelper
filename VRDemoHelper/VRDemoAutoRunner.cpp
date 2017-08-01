#include "stdafx.h"
#include "VRDemoAutoRunner.h"

#include "VRDemoConfigurator.h"
#include "VRDemoHelper.h"
#include "log4cplus\log4cplus.h"
#include <sstream>

static std::string REG_SUB_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static std::string REG_ITEM = "VRDemoHelper";

VRDemoAutoRunner::VRDemoAutoRunner()
{
}


VRDemoAutoRunner::~VRDemoAutoRunner()
{
}


void VRDemoAutoRunner::setup(VRDemoConfigurator& configurator) {
    bool autoRun;
    log4cplus::Logger logger = log4cplus::Logger::getRoot();

    if (configurator.getValue(HELPER_SECTION_NAME, "AutoRun", &autoRun)) {
        HKEY key;
        if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_CURRENT_USER, REG_SUB_KEY.c_str(), 0, KEY_WRITE, &key)) {
            if (autoRun) {
                char path[MAX_PATH];
                std::ostringstream strBuf;
                if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
                    strBuf << "\"" << path << "\"";
                    if (ERROR_SUCCESS == RegSetValueExA(key, REG_ITEM.c_str(), 0, REG_SZ, (unsigned char *)strBuf.str().c_str(), (DWORD)strBuf.str().length()+1)) {
                        LOG4CPLUS_INFO(logger, "Set AutoRun true");
                    }
                }
            }
            else {
                if (ERROR_SUCCESS == RegDeleteKeyValueA(key, NULL, REG_ITEM.c_str())) {
                    LOG4CPLUS_INFO(logger, "Set AutoRun false");
                }
            }
            RegCloseKey(key);
        }
    }
}
