// VRDemoCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "VRDemoCore.h"

#include "util/l4util.h"
#include "VRDemoArbiter.h"
#include "VRDemoConfigurator.h"

#define HELPER_NAME "VRDemoHelper"				// used to tell if we're in helper process

// All instances share data segment
#pragma data_seg(".shared")
CHAR szConfigFilePath[MAX_PATH] = "";
VRDemoArbiter::Toggles toggles = { false, true, true, true };
#pragma data_seg()
#pragma comment(linker,"/section:.shared,rws")

BOOL fnIsHelperProcess()
{
    static BOOL bIsHelperProcess = FALSE;
    static BOOL bCalled = FALSE;
    if (!bCalled) {
        CHAR szFileName[MAX_PATH];
        if (0 < GetModuleFileNameA(NULL, szFileName, MAX_PATH))
        {
            bCalled = TRUE;
            if (strstr(szFileName, HELPER_NAME))
            {
                bIsHelperProcess = TRUE;
            }
        }
    }
    return bIsHelperProcess;
}

void fnDelayInit()
{
    // We do nothing when loaded by helper process(not necessary, either),
    if (!fnIsHelperProcess()) {
        if (VRDemoConfigurator::getInstance().init(szConfigFilePath)) {
            VRDemoArbiter::getInstance().init(toggles);
        }
    }
}

// 这是导出函数的一个示例。
LRESULT WINAPI fnWndMsgProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    static BOOL bDelayInited = false;

    if (!bDelayInited) {
        fnDelayInit();
        bDelayInited = TRUE;
    }

	// we don't handle hook messages in helper process
	if (!fnIsHelperProcess() && !toggles.m_pause)
	{
		switch (nCode)
		{
		case HCBT_CREATEWND:
		case HCBT_ACTIVATE:
		{
			VRDemoArbiter::getInstance().arbitrate(VRDemoArbiter::RT_MESSAGE, nCode, (HWND)wParam);
		}
		break;
		default:
			break;
		}
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

bool WINAPI fnInit(const char *szConfigFilePath_, const VRDemoArbiter::Toggles& toggles_)
{
	strcpy_s(szConfigFilePath, MAX_PATH, szConfigFilePath_);
	// TODO: local config file content into share data segment
    toggles = toggles_;
	return true;
}

void WINAPI fnSetToggleValue(int nIndex, bool bValue)
{
    if (VRDemoArbiter::TI_MIN <= nIndex && VRDemoArbiter::TI_MAX >= nIndex) {
        toggles.m_values[nIndex] = bValue;
    }
}

