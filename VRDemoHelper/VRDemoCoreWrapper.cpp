#include "stdafx.h"
#include "VRDemoCoreWrapper.h"

#include <log4cplus\log4cplus.h>

#include "util\l4util.h"
#include "VRDemoArbiter.h"

#ifdef _WIN64
const std::string VRDemoCoreWrapper::FILE_HOOK_DLL = "VRDemoCore_x64.dll"; 
#else
const std::string VRDemoCoreWrapper::FILE_HOOK_DLL = "VRDemoCore_x86.dll";
#endif
const std::string VRDemoCoreWrapper::FILE_SETTINGS = "settings.ini";
const std::string VRDemoCoreWrapper::FUNCTION_INIT = "Init";
const std::string VRDemoCoreWrapper::FUNCTION_HOOK_PROC_CBT = "CBTProc";
const std::string VRDemoCoreWrapper::FUNCTION_SET_TOGGLE = "SetToggle";

VRDemoCoreWrapper::VRDemoCoreWrapper():
    m_dll(nullptr),
    m_hook(nullptr),
    m_setToggleFunc(nullptr)
{
}


VRDemoCoreWrapper::~VRDemoCoreWrapper()
{
    uninit();
}

bool VRDemoCoreWrapper::init(const VRDemoArbiter::Toggles& toggles)
{
    bool result = false;
    log4cplus::Logger logger = log4cplus::Logger::getRoot();

    m_dll = LoadLibrary(l4util::getFileFullPath(FILE_HOOK_DLL).c_str());
    if (m_dll) {
        InitFuncPtr initFunc = (InitFuncPtr)GetProcAddress(m_dll, FUNCTION_INIT.c_str());
        HOOKPROC hookProc = (HOOKPROC)GetProcAddress(m_dll, FUNCTION_HOOK_PROC_CBT.c_str());
        SetToggleFuncPtr setToggleFunc = (SetToggleFuncPtr)GetProcAddress(m_dll, FUNCTION_SET_TOGGLE.c_str());

        if (initFunc && hookProc && setToggleFunc) {
            if (initFunc(l4util::getCurrentExePath().c_str(), toggles)) {
                m_hook = SetWindowsHookEx(
                    WH_CBT,
                    hookProc,
                    m_dll,
                    0
                );
                if (m_hook) {
                    m_setToggleFunc = setToggleFunc;

                    VRDemoEventDispatcher::getInstance().addEventListener(
                        VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED,
                        VRDemoEventDispatcher::VRDemoEventListenerPtr(this)
                    );

                    m_windowMessageToggleBenchmark = RegisterWindowMessageA(VR_DEMO_WINDOW_MESSAGE_TOGGLE_BENCHMARK);
                    result = true;
                }
                else {
                    LOG4CPLUS_ERROR(logger, "Failed to hook, errno: " << GetLastError());
                }
            }
            else {
                LOG4CPLUS_ERROR(logger, "Failed to init in core module");
            }
        }
        else {
            LOG4CPLUS_ERROR(logger, "Invalid core exports, check your dll version");
        }
    }
    if (!result) {
        uninit();
    }
    return result;
}

void VRDemoCoreWrapper::handleEvent(int event, unsigned long long param1, unsigned long long param2)
{
    bool value = 0 != param2;
    switch (event) {
    case VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED:
        m_setToggleFunc(static_cast<int>(param1), value);
        break;
    default:
        break;
    }
}

bool VRDemoCoreWrapper::toggleBenchmark()
{
    HWND wnd = GetForegroundWindow();
    
    if (wnd) {
        SendMessageA(wnd, m_windowMessageToggleBenchmark, 0, 0);
    }
    return false;
}


void VRDemoCoreWrapper::uninit()
{
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
    if (m_dll) {
        FreeLibrary(m_dll);
        m_dll = nullptr;
    }
}
