#pragma once
#include <VRDemoEventDispatcher.h>
#include <log4cplus\helpers\pointer.h>

class VRDemoCoreWrapper : public IVRDemoEventListener
{
public:   
    typedef log4cplus::helpers::SharedObjectPtr<VRDemoCoreWrapper> VRDemoCoreWrapperPtr;
    VRDemoCoreWrapper();
    ~VRDemoCoreWrapper();
    bool init(BOOL trace);
    void handleEvent(int event, unsigned long long param);
private:
    typedef BOOL(*InitFuncPtr)(const CHAR *, BOOL);
    typedef VOID(*SetPauseFuncPtr)(BOOL);

    HINSTANCE m_dll;
    HHOOK m_hook;

    SetPauseFuncPtr m_setPauseFunc;

    static const std::string FILE_HOOK_DLL;
    static const std::string FILE_RULE_CONFIG;
    static const std::string FUNCTION_INIT;
    static const std::string FUNCTION_HOOK_PROC;
    static const std::string FUNCTION_SET_PAUSE;
};

