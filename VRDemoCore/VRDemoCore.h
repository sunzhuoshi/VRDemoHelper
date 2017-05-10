#pragma once
#include "VRDemoArbiter.h"                        

extern "C" LRESULT WINAPI CBTProc(INT nCode, WPARAM wParam, LPARAM lParam);

extern "C" LRESULT WINAPI CallWndProc(INT nCode, WPARAM wParam, LPARAM lParam);

extern "C" bool WINAPI Init(const char *rootPath, const VRDemoArbiter::Toggles& toggles);

extern "C" void WINAPI SetToggle(int index, bool value);
