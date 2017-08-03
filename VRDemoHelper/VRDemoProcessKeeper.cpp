#include "stdafx.h"
#include "VRDemoProcessKeeper.h"
#include <log4cplus\log4cplus.h>
#include "VRDemoHelper.h"

#define VR_DEMO_HELPER_X64 "VRDemoHelper.dll" // use .dll extension to avoid running by user directly

#ifdef _WIN64
#define MODE_CHILD
#else
#define MODE_PARENT
#endif 

extern HINSTANCE hInst;
#include "Resource.h"

VRDemoProcessKeeper::VRDemoProcessKeeper()
{
}


VRDemoProcessKeeper::~VRDemoProcessKeeper()
{
}

bool VRDemoProcessKeeper::init(DWORD parentProcessID)
{
    m_logger = log4cplus::Logger::getRoot();
#ifdef MODE_PARENT
    if (!createChildProcess()) {
        return false;
    }
#else
    m_parentProcessID = parentProcessID;
#endif // MODE_PARENT
    m_thread = std::thread(&VRDemoProcessKeeper::run, this);
    return true;
}

void VRDemoProcessKeeper::uninit()
{
#ifdef MODE_PARENT
    if (m_childProcessHandle) {
        TerminateProcess(m_childProcessHandle, 0);
        WaitForSingleObject(m_childProcessHandle, 0);
    }
#else
#endif // MODE_PARENT
    m_runFlag = false;
    m_thread.join();
}

void VRDemoProcessKeeper::sendMessageToChildProcess(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (m_childProcessHandle) {
        SendMessageA(m_childProcessMainWindow, Msg, wParam, lParam);
    }
}

bool VRDemoProcessKeeper::createChildProcess()
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    std::ostringstream cmdBuf;
    cmdBuf << "\"" << l4util::getCurrentExePath() << VR_DEMO_HELPER_X64 << "\" -b -p" << GetCurrentProcessId();
    char cmd[MAX_PATH];
    strncpy_s(cmd, cmdBuf.str().c_str(), sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = 0;
    LOG4CPLUS_INFO(m_logger, "Creating child process...");
    if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        m_childProcessID = pi.dwProcessId;
        m_childProcessHandle = pi.hProcess;
        LOG4CPLUS_INFO(m_logger, "Child process created");
        LOG4CPLUS_DEBUG(m_logger, "Waiting child process initialization...");
        DWORD ret = WaitForInputIdle(pi.hProcess, INFINITE);
        LOG4CPLUS_DEBUG(m_logger, "Waiting result: " << ret);
        findChildProcessMainWindow();
        return true;
    }
    else {
        LOG4CPLUS_ERROR(m_logger, "Failed to create child process, error: " << GetLastError());
        return false;
    }
}

void VRDemoProcessKeeper::run()
{
    // TODO: use signal to speed up quit
    while (m_runFlag) {
#ifdef MODE_PARENT
        if (!ifProcessRunning(m_childProcessID)) {
            LOG4CPLUS_ERROR(m_logger, "Child process is not running, maybe crashed");
            CloseHandle(m_childProcessHandle);
            if (!createChildProcess()) {
                LOG4CPLUS_ERROR(m_logger, "Exit parent process");
                ExitProcess(-1);
            }
        }
#else
        if (!ifProcessRunning(m_parentProcessID)) {
            LOG4CPLUS_ERROR(m_logger, "Parent process is not running, exit child process");
            ExitProcess(-1);
        }
#endif
        Sleep(VRDemoProcessKeeper::CHECK_INTERVAL);
    }
}

bool VRDemoProcessKeeper::ifProcessRunning(DWORD processID)
{
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (processHandle) {
        CloseHandle(processHandle);
        return true;
    }
    else {
        return false;
    }
}

void VRDemoProcessKeeper::findChildProcessMainWindow()
{
    char szWindowClass[HELPER_MAX_LOADSTRING];
    char szTitle[HELPER_MAX_LOADSTRING];

    LoadStringA(hInst, IDS_APP_TITLE, szTitle, sizeof(szTitle));
    strcat_s(szTitle, sizeof(szTitle), HELPER_WINDOW_TITLE_CHILD_PROCESS_POSTFIX);
    LoadStringA(hInst, IDC_VRDEMOHELPER, szWindowClass, sizeof(szWindowClass));

    m_childProcessMainWindow = FindWindowEx(NULL, NULL, szWindowClass, szTitle);
}
