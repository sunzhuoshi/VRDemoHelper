#pragma once
#include "VRDemoArbiter.h"                        

extern "C" LRESULT WINAPI fnWndMsgProc(INT nCode, WPARAM wParam, LPARAM lParam);

extern "C" bool WINAPI fnInit(const char *szRuleConfigFilePath, const VRDemoArbiter::Toggles& toggles);

extern "C" void WINAPI fnSetToggleValue(int nIndex, bool bValue);
