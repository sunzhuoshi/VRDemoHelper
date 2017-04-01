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
    typedef VOID(*SetMaximizeGamesFuncPtr)(BOOL);
    typedef VOID(*SetHideSteamVrNotificationFuncPtr)(BOOL);

    HINSTANCE m_dll;
    HHOOK m_hook;

    SetPauseFuncPtr m_setPauseFunc;
    SetMaximizeGamesFuncPtr m_setMaximizeGamesFunc;
    SetHideSteamVrNotificationFuncPtr m_setHideSteamVrNotificationFunc;

    static const std::string FILE_HOOK_DLL;
    static const std::string FILE_SETTINGS;
    static const std::string FUNCTION_INIT;
    static const std::string FUNCTION_HOOK_PROC;
    static const std::string FUNCTION_SET_PAUSE;
    static const std::string FUNCTION_SET_MAXIMIZE_GAMES;
    static const std::string FUNCTION_SET_HIDE_STEAM_VR_NOTIFICATION;
};

