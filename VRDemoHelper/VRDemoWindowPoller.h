#pragma once
#include <log4cplus\thread\threads.h>

/*
  Rule type "POLL" owner
*/
class VRDemoWindowPoller : public log4cplus::thread::AbstractThread
{
public:
    typedef log4cplus::helpers::SharedObjectPtr<VRDemoWindowPoller> VRDemoWindowPollerPtr;
    VRDemoWindowPoller();
    ~VRDemoWindowPoller();
    bool init();
    void run();
    void stop();
    void pause();
    void resume();
    bool isPaused() {
        return m_pauseFlag;
    }
    void setPause(bool pause) {
        m_pauseFlag = pause;
    }
    static const int POLL_INTERVAL = 1000; // ms
private:
    static BOOL CALLBACK enumChildProc(HWND wnd, LPARAM param);
    BOOL realEnumChildProc(HWND wnd);
    bool m_runFlag;
    bool m_pauseFlag;
    log4cplus::thread::ManualResetEvent m_event;
};

