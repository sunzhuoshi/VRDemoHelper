#include "stdafx.h"
#include "VRDemoHotKeyManager.h"


#include "util\l4util.h"
#include "VRDemoConfigurator.h"
#include "VRDemoTogglesWrapper.h"

const std::string VRDemoHotKeyManager::HOT_KEY_PAUSE = "HotKeyPause";

VRDemoHotKeyManager::VRDemoHotKeyManager()
{
}

VRDemoHotKeyManager::~VRDemoHotKeyManager()
{
}

void VRDemoHotKeyManager::configurate(HWND wnd)
{
    // TODO: to make it configuable
    /*
    const VRDemoConfigurator::KeyValueMap *helperSection = VRDemoConfigurator::getInstance().getHelperSection();
    if (helperSection) {
        VRDemoConfigurator::KeyValueMap::const_iterator it = helperSection->find(l4util::toUpper(HOT_KEY_PAUSE));
        std::string keyPause;
        if (it != helperSection->end()) {
            keyPause = it->second;
        }
    }
    */
    RegisterHotKey(wnd, 1, 0, VK_F8);   // pause
    RegisterHotKey(wnd, 2, 0, VK_F6);   // benchmark
}
