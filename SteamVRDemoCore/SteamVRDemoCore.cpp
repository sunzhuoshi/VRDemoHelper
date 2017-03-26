// SteamVRDemoCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SteamVRDemoCore.h"

#include <log4cplus/log4cplus.h>
#include "SteamVRDemoUtil.h"
#include "SteamVRDemoRuleManager.h"

#define HELPER_NAME "SteamVRDemoHelper"				// used to tell if we're in helper process

// 这是导出变量的一个示例
BOOL IsHelperProcess = FALSE;

#pragma data_seg(".shared")
STEAMVRDEMOCORE_API CHAR szConfigFilePath[MAX_PATH] = "";
STEAMVRDEMOCORE_API BOOL bActive = TRUE;
#pragma data_seg()
#pragma comment(linker,"/section:.shared,rws")

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
	//result = false; // debug hack
	return result;
}

void fnInitLog()
{
	log4cplus::SharedAppenderPtr append_1(new log4cplus::SocketAppender("127.0.0.1", 8888, "127.0.0.1"));
	append_1->setName(LOG4CPLUS_TEXT("First"));
	log4cplus::Logger::getInstance("SERVER").addAppender(append_1);
}

void fnInit()
{
	IsHelperProcess = fnIsHelperProcess();
	// We don't log messages when loaded by helper process,
	// or it will cause file dead lock, for client and server will try to use the same SERVER file apender 
	if (!IsHelperProcess) {
		fnInitLog();
		log4cplus::Logger logger = log4cplus::Logger::getInstance("SERVER");
		if (!SteamVRDemoRuleManager::init(szConfigFilePath)) {
			LOG4CPLUS_ERROR(logger, "Failed to parse rule config file: " << szConfigFilePath << std::endl);
		}
	}
}

// 这是导出函数的一个示例。
STEAMVRDEMOCORE_API LRESULT WINAPI fnWndMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	char szClsName[MAX_PATH];
	HWND hWnd = NULL;
	static bool inited = false;

	if (!inited) {
		fnInit();
		inited = true;
	}

	// we don't handle hook messages in helper process
	if (!IsHelperProcess)
	{
		log4cplus::Logger logger = log4cplus::Logger::getInstance("SERVER");
		DWORD processId = GetCurrentProcessId();
		std::string processName = steam_vr_demo_helper::getCurrentProcessName();

		switch (nCode)
		{
		case HCBT_CREATEWND:
		{
			hWnd = (HWND)wParam;
			RealGetWindowClassA(hWnd, szClsName, MAX_PATH);
			SteamVRDemoRuleManager::handleMessage(nCode, (HWND)wParam);
			LOG4CPLUS_INFO(logger, "[CREATED] ID=" << processId << ", name=" << processName << ", Class=" << szClsName << std::endl);
		}
		break;
		case HCBT_ACTIVATE:
		{
			SteamVRDemoRuleManager::handleMessage(nCode, (HWND)wParam);
			hWnd = (HWND)wParam;
			RealGetWindowClassA(hWnd, szClsName, MAX_PATH);
			SteamVRDemoRuleManager::handleMessage(nCode, (HWND)wParam);
			LOG4CPLUS_INFO(logger, "[ACTIVATED] ID=" << processId << ", name=" << processName << ", Class=" << szClsName << std::endl);
		}
		break;
		default:
			break;
		}
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

STEAMVRDEMOCORE_API void WINAPI fnSetRuleConfigFilePath(const char *szFilePath)
{
	strcpy_s(szConfigFilePath, MAX_PATH, szFilePath);
}
