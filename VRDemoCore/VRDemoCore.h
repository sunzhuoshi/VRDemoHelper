#ifdef VRDEMOCORE_EXPORTS
#define VRDEMOCORE_API __declspec(dllexport)
#else
#define VRDEMOCORE_API __declspec(dllimport)
#endif

#include "VRDemoArbiter.h"

extern "C" LRESULT WINAPI fnWndMsgProc(INT nCode, WPARAM wParam, LPARAM lParam);

extern "C" bool WINAPI fnInit(const char *szRuleConfigFilePath);

extern "C" void WINAPI fnSetToggleValue(int nIndex, bool bValue);
