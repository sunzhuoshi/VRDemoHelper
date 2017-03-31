#include "stdafx.h"
#include "VRDemoCoreWrapper.h"

#include "util\l4util.h"
#include <WinUser.h>

#ifdef _WIN64
const std::string VRDemoCoreWrapper::FILE_HOOK_DLL = "VRDemoCore.dll"; // TODO: rename to "VRDemoCore_x64.dll"
#else
const std::string VRDemoCoreWrapper::FILE_HOOK_DLL = "VRDemoCore_x86.dll";
#endif
const std::string VRDemoCoreWrapper::FILE_RULE_CONFIG = "rule_config.ini";
const std::string VRDemoCoreWrapper::FUNCTION_INIT = "fnInit";
const std::string VRDemoCoreWrapper::FUNCTION_HOOK_PROC = "fnWndMsgProc";
const std::string VRDemoCoreWrapper::FUNCTION_SET_PAUSE = "fnSetPause";

VRDemoCoreWrapper::VRDemoCoreWrapper():
    m_dll(nullptr),
    m_hook(nullptr),
    m_setPauseFunc(nullptr)
{
}


VRDemoCoreWrapper::~VRDemoCoreWrapper()
{
    UnhookWindowsHookEx(m_hook);
    FreeLibrary(m_dll);
}

bool VRDemoCoreWrapper::init(BOOL trace)
{
    bool result = false;
    m_dll = LoadLibrary(l4util::getFileFullPath(FILE_HOOK_DLL).c_str());
    if (m_dll) {
        InitFuncPtr initFunc = (InitFuncPtr)GetProcAddress(m_dll, FUNCTION_INIT.c_str());
        HOOKPROC hookProc = (HOOKPROC)GetProcAddress(m_dll, FUNCTION_HOOK_PROC.c_str());
        SetPauseFuncPtr setPauseFunc = (SetPauseFuncPtr)GetProcAddress(m_dll, FUNCTION_SET_PAUSE.c_str());

        if (initFunc && hookProc && setPauseFunc) {
            if (initFunc(l4util::getFileFullPath(FILE_RULE_CONFIG).c_str(), trace)) {
                m_hook = SetWindowsHookEx(
                    WH_CBT,
                    hookProc,
                    m_dll,
                    0
                );
                if (m_hook) {
                    m_setPauseFunc = setPauseFunc;
                    VRDemoEventDispatcher::getInstance().addEventListener(
                        VRDemoEventDispatcher::EV_PAUSE_CHANGED,
                        VRDemoEventDispatcher::VRDemoEventListenerPtr(this)
                    );
                    result = true;
                }
                else {
                    // TODO: log hook error
                }
            }
            else {
                // TODO: log init error       
            }
        }
        else {
            // TODO: log get func error
        }
    }
    return result;
}

void VRDemoCoreWrapper::handleEvent(int event, unsigned long long param)
{
    switch (event) {
    case VRDemoEventDispatcher::EV_PAUSE_CHANGED:
        m_setPauseFunc(0 != param);
        break;
    default:
        break;
    }
}
