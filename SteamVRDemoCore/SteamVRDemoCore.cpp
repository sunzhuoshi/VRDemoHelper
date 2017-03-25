// SteamVRDemoCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SteamVRDemoCore.h"

#include <log4cplus/log4cplus.h>

// 这是导出变量的一个示例
STEAMVRDEMOCORE_API int nActive=0;

/*
#pragma data_seg(".shared")
HHOOK hHook = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:.shared,rws")
*/


STEAMVRDEMOCORE_API void WINAPI fnInit()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	log4cplus::SharedAppenderPtr append_1(new log4cplus::SocketAppender("127.0.0.1", 8888, "127.0.0.1"));
	append_1->setName(LOG4CPLUS_TEXT("First"));
	log4cplus::Logger::getRoot().addAppender(append_1);

	log4cplus::Logger root = log4cplus::Logger::getRoot();
	LOG4CPLUS_INFO(root, "test dll");
}

// 这是导出函数的一个示例。
STEAMVRDEMOCORE_API LRESULT WINAPI fnWndMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	char tmp[MAX_PATH];
	HWND hWnd = NULL;
	log4cplus::Logger logger = log4cplus::Logger::getRoot();

	switch (nCode) 
	{
	case HCBT_CREATEWND:
	{
		hWnd = (HWND)wParam;
		CBT_CREATEWND *createWndParam = (CBT_CREATEWND *)lParam;
		//ShowWindow(hWnd, SW_MAXIMIZE);
	}
		break;
	case HCBT_ACTIVATE: 
	{
		hWnd = (HWND)wParam;
		CBTACTIVATESTRUCT *activeParam = (CBTACTIVATESTRUCT *)lParam;
		RealGetWindowClassA(hWnd, tmp, MAX_PATH);
		LOG4CPLUS_INFO(logger, tmp);
		//(hWnd, SW_MAXIMIZE);
	}
		break;
	default:
		break;
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
