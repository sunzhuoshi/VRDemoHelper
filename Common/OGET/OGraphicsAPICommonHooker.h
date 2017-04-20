#pragma once

#include <thread>

#include "OIGraphicsAPIHooker.h"

class OGraphicsAPICommonHooker : public OIGraphicsAPIHooker
{
public:
    OGraphicsAPICommonHooker();
    virtual ~OGraphicsAPICommonHooker();
    virtual void hook();
protected:
    virtual void hookWithWindow(HWND wnd) = 0;
private:
    void run();
    static DWORD WINAPI threadProc(LPVOID param);
    static BOOL CALLBACK enumWindowsProc(HWND wnd, LPARAM param);
    // TODO: find why std::thread will cause dead lock here 
    HANDLE m_hookThreadHandle;
};

