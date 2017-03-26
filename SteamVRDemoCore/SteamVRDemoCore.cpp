// SteamVRDemoCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SteamVRDemoCore.h"

#include <log4cplus/log4cplus.h>



#define HELPER_NAME "SteamVRDemoHelper"				// used to tell if we're in helper process

// 这是导出变量的一个示例
STEAMVRDEMOCORE_API int nActive=0;

BOOL IsHelperProcess = FALSE;

/*
#pragma data_seg(".shared")
HHOOK hHook = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:.shared,rws")
*/

BOOL fnIsHelperProcess()
{
	BOOL result = FALSE;
	CHAR szFileName[MAX_PATH];
	if (0 < GetModuleFileNameA(NULL, szFileName, MAX_PATH))
	{
		if (strstr(szFileName, HELPER_NAME))
		{
			result = TRUE;
		}
	}
	return result;
}

void fnInit()
{
	IsHelperProcess = fnIsHelperProcess();
	// We don't log messages when loaded by helper process
	if (!IsHelperProcess) {
		log4cplus::SharedAppenderPtr append_1(new log4cplus::SocketAppender("192.168.1.2", 8888, "192.168.1.2"));
		append_1->setName(LOG4CPLUS_TEXT("First"));
		log4cplus::Logger::getRoot().addAppender(append_1);
	}
}

// 这是导出函数的一个示例。
STEAMVRDEMOCORE_API LRESULT WINAPI fnWndMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	char szMsg[MAX_PATH];
	HWND hWnd = NULL;
	static bool inited = false;

	if (!inited) {
		fnInit();
		inited = true;
	}

	// we don't handle hook messages in helper process
	if (!IsHelperProcess)
	{
		log4cplus::Logger logger = log4cplus::Logger::getRoot();
		switch (nCode)
		{
		case HCBT_CREATEWND:
		{
			hWnd = (HWND)wParam;
			CBT_CREATEWND *createWndParam = (CBT_CREATEWND *)lParam;
			RealGetWindowClassA(hWnd, szMsg, MAX_PATH);
			strcat_s(szMsg, " CREATED");
			LOG4CPLUS_INFO(logger, szMsg);
		}
		break;
		case HCBT_ACTIVATE:
		{
			hWnd = (HWND)wParam;
			CBTACTIVATESTRUCT *activeParam = (CBTACTIVATESTRUCT *)lParam;
			RealGetWindowClassA(hWnd, szMsg, MAX_PATH);
			strcat_s(szMsg, " ACTIVATED");
			LOG4CPLUS_INFO(logger, szMsg);
		}
		break;
		default:
			break;
		}
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
