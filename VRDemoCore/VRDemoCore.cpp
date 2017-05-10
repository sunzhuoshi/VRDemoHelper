// VRDemoCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "VRDemoCore.h"

#include "util/l4util.h"
#include "VRDemoArbiter.h"
#include "VRDemoConfigurator.h"
#include "OGET\OBenchmarker.h"

#define HELPER_NAME "VRDemoHelper"				// used to tell if we're in helper process

// All instances share data segment
#pragma data_seg(".shared")
char g_rootPath[MAX_PATH] = "";
// it is registered in system, so don't worry about conflit with other messages
UINT g_windowMessageToggleBenchmark = 0;
VRDemoArbiter::Toggles g_toggles = { false, true, true, true };
#pragma data_seg()
#pragma comment(linker,"/section:.shared,rws")

bool IsHelperProcess()
{
    static bool isHelperProcess = false;
    static BOOL called = false;
    if (!called) {
        char fileName[MAX_PATH];
        if (0 < GetModuleFileNameA(NULL, fileName, MAX_PATH))
        {
            called = TRUE;
            if (strstr(fileName, HELPER_NAME))
            {
                isHelperProcess = true;
            }
        }
    }
    return isHelperProcess;
}

void fnDelayInit()
{
    // We do nothing when loaded by helper process(not necessary, either),
    if (!IsHelperProcess()) {
        if (VRDemoConfigurator::getInstance().init(g_rootPath + VRDemoConfigurator::FILE_SETTINGS)) {
            VRDemoArbiter::getInstance().init(g_toggles);
            OBenchmarker::getInstance().init(g_toggles);
        }
    }
}

LRESULT WINAPI CallWndProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    if (HC_ACTION == nCode) {
        PCWPSTRUCT params = (PCWPSTRUCT)lParam;
        if (params->message == g_windowMessageToggleBenchmark) {
            OBenchmarker::getInstance().toggle();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT WINAPI CBTProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    static BOOL delayInited = false;

    if (!delayInited) {
        fnDelayInit();
        delayInited = TRUE;
    }

	// we don't handle hook messages in helper process
	if (!IsHelperProcess() && !g_toggles.m_pause)
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

bool WINAPI Init(const char *rootPath, const VRDemoArbiter::Toggles& toggles)
{
	strcpy_s(g_rootPath, MAX_PATH, rootPath);
    g_toggles = toggles;
    g_windowMessageToggleBenchmark = RegisterWindowMessageA(VR_DEMO_WINDOW_MESSAGE_TOGGLE_BENCHMARK);
	return true;
}

void WINAPI SetToggle(int index, bool value)
{
    if (VRDemoArbiter::TI_MIN <= index && VRDemoArbiter::TI_MAX >= index) {
        g_toggles.m_values[index] = value;
    }
}

