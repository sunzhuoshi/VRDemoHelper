#include "stdafx.h"
#include "VRDemoWindowPoller.h"

#include <log4cplus\log4cplus.h>
#include <Windows.h>
#include "VRDemoArbiter.h"
#include "util\l4util.h"

VRDemoWindowPoller::VRDemoWindowPoller() :
    m_runFlag(true),
    m_pauseFlag(false)
{
    VRDemoEventDispatcher::getInstance().addEventListener(
        VRDemoEventDispatcher::EV_PAUSE_CHANGED, 
        VRDemoEventDispatcher::VRDemoEventListenerPtr(this)
    );
}


VRDemoWindowPoller::~VRDemoWindowPoller()
{
}

bool VRDemoWindowPoller::init()
{
    bool result = false;

    if (VRDemoArbiter::getInstance().init(
        l4util::getFileFullPath(VRDemoArbiter::FILE_SETTINGS),
        VR_DEMO_LOGGER_CLIENT
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
        if (m_runFlag && !m_pauseFlag) {
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

void VRDemoWindowPoller::pause()
{
    m_pauseFlag = true;
}

void VRDemoWindowPoller::resume()
{
    m_pauseFlag = false;
}

void VRDemoWindowPoller::handleEvent(int event, unsigned long long param)
{
    bool flag = 0 != param;
    switch (event) {
    case VRDemoEventDispatcher::Event::EV_PAUSE_CHANGED:
        m_pauseFlag = flag;
        break;
    case VRDemoEventDispatcher::EV_HIDE_STEAM_VR_NOTIFICATION_CHANGED:
        VRDemoArbiter::getInstance().setHideSteamVrNotification(flag);
        break;
    case VRDemoEventDispatcher::EV_MAXIMIZE_GAMES_CHANGED:
        VRDemoArbiter::getInstance().setMaximizeGames(flag);
        break;
    default:
        break;
    }
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

        GetWindowThreadProcessId(wnd, &processId);
        // handle other processes only
        if (GetCurrentProcessId() != processId) {
            VRDemoArbiter::getInstance().arbitrate(VRDemoArbiter::RT_POLL, VRDemoArbiter::RM_UNKNOWN, wnd);
        }
        cont = TRUE;
    }
    return cont;
}

