#include "stdafx.h"
#include "VRDemoWindowPoller.h"

#include <log4cplus\log4cplus.h>
#include <Windows.h>
#include "VRDemoArbiter.h"
#include "util\l4util.h"

VRDemoWindowPoller::VRDemoWindowPoller() :
    m_runFlag(true)
{
}


VRDemoWindowPoller::~VRDemoWindowPoller()
{
}

bool VRDemoWindowPoller::init()
{
    bool result = false;
    log4cplus::Logger serverLogger = log4cplus::Logger::getInstance("CLIENT");

    if (VRDemoArbiter::getInstance().init(
        l4util::getFileFullPath(VRDemoArbiter::RULE_CONFIG_FILE),
        "CLIENT"
    )) {
        start();
        result = true;
    }
    return result;
}

void VRDemoWindowPoller::run()
{
    while (m_runFlag)
    {
        m_event.timed_wait(POLL_INTERVAL);
        if (m_runFlag) {
            EnumWindows(&VRDemoWindowPoller::enumChildProc, (LPARAM)this);
        }
    }
}

void VRDemoWindowPoller::stop()
{
    m_runFlag = false;
    m_event.signal();
    this->join();
}

BOOL CALLBACK VRDemoWindowPoller::enumChildProc(HWND wnd, LPARAM param)
{
    VRDemoWindowPoller *poller = (VRDemoWindowPoller *)param;
    return poller->realEnumChildProc(wnd);
}

BOOL VRDemoWindowPoller::realEnumChildProc(HWND wnd)
{
    BOOL cont = FALSE;
    if (m_runFlag) {
        DWORD processId;
        HWND ownerWindow;

        GetWindowThreadProcessId(wnd, &processId);
        // NOTE: DO NOT use GetParent(wnd), use GetWindow(wnd, GW_OWNER) instead
        if (GetCurrentProcessId() != processId) {
            VRDemoArbiter::getInstance().arbitrate(VRDemoArbiter::RT_POLL, VRDemoArbiter::RM_UNKNOWN, wnd);
        }
        cont = TRUE;
    }
    return cont;
}

