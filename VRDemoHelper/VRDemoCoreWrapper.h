#pragma once
#include <log4cplus\helpers\pointer.h>

#include "VRDemoArbiter.h"
#include "VRDemoEventDispatcher.h"

class VRDemoCoreWrapper : public IVRDemoEventListener
{
public:   
    typedef log4cplus::helpers::SharedObjectPtr<VRDemoCoreWrapper> VRDemoCoreWrapperPtr;
    VRDemoCoreWrapper();
    ~VRDemoCoreWrapper();
    bool init(const VRDemoArbiter::Toggles& toggles);
    void handleEvent(int event, unsigned long long param1, unsigned long long param2);
    bool toggleBenchmark();
private:
    typedef bool (WINAPI *InitFuncPtr)(const char *, const VRDemoArbiter::Toggles&);
    typedef void (WINAPI *SetToggleFuncPtr)(int, bool);

    void uninit();

    HINSTANCE m_dll = NULL;
    HHOOK m_hook = NULL;
    UINT m_windowMessageToggleBenchmark = 0;

    SetToggleFuncPtr m_setToggleFunc = nullptr;

    static const std::string FILE_HOOK_DLL;
    static const std::string FILE_SETTINGS;
    static const std::string FUNCTION_INIT;
    static const std::string FUNCTION_HOOK_PROC_CBT;
    static const std::string FUNCTION_SET_TOGGLE;
};

