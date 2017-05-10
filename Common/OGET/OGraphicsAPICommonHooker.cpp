#include "stdafx.h"
#include "OGraphicsAPICommonHooker.h"

#include "OInlineHookUtil.h"


OGraphicsAPICommonHooker::OGraphicsAPICommonHooker()
{
    OInlineHookUtil::init();
}

OGraphicsAPICommonHooker::~OGraphicsAPICommonHooker()
{
    if (m_hookThreadHandle) {
        WaitForSingleObject(m_hookThreadHandle, INFINITE);
        CloseHandle(m_hookThreadHandle);
    }
    OInlineHookUtil::unint();
}


void OGraphicsAPICommonHooker::hook()
{
    m_hookThreadHandle = CreateThread(NULL, 0, OGraphicsAPICommonHooker::threadProc, this, 0, NULL);
}

void OGraphicsAPICommonHooker::run()
{
    HWND wnd = GetForegroundWindow();

    if (wnd) {
        hookWithWindow(wnd);
    }
    else {
        EnumWindows(&OGraphicsAPICommonHooker::enumWindowsProc, reinterpret_cast<LPARAM>(this));
    }
}

DWORD WINAPI OGraphicsAPICommonHooker::threadProc(LPVOID param)
{
    OGraphicsAPICommonHooker *obj = reinterpret_cast<OGraphicsAPICommonHooker *>(param);
    obj->run();
    return TRUE;
}

BOOL CALLBACK OGraphicsAPICommonHooker::enumWindowsProc(HWND wnd, LPARAM param)
{
    BOOL cont = TRUE;
    DWORD windowProcessID;
    OGraphicsAPICommonHooker *hooker = reinterpret_cast<OGraphicsAPICommonHooker *>(param);

    GetWindowThreadProcessId(wnd, &windowProcessID);
    if (GetCurrentProcessId() == windowProcessID) {
        cont = FALSE;
        hooker->hookWithWindow(wnd);
    }
    return cont;
}
