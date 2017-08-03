#pragma once

#include <thread>
#include "util\L4Singleton.hpp"
#include <log4cplus\log4cplus.h>

class VRDemoProcessKeeper : public L4Singleton<VRDemoProcessKeeper>
{
public:
    static const int CHECK_INTERVAL = 100; // ms
    VRDemoProcessKeeper();
    ~VRDemoProcessKeeper();
    bool init(DWORD parentProcessID);
    void uninit();
    void sendMessageToChildProcess(UINT Msg, WPARAM wParam, LPARAM lParam);
private:
    bool createChildProcess();
    void run();
    bool ifProcessRunning(DWORD processID);
    void findChildProcessMainWindow();
private:
    std::thread m_thread;
    DWORD m_parentProcessID;
    DWORD m_childProcessID;
    HANDLE m_childProcessHandle;
    HWND m_childProcessMainWindow;
    log4cplus::Logger m_logger;
};

