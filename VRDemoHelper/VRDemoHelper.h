#pragma once

#include "resource.h"

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

#define HELPER_SECTION_NAME "VRDemoHelper"

// to find message only window of child process, we have to use a differient window title
// or it will return parent process window
#define HELPER_WINDOW_TITLE_CHILD_PROCESS_POSTFIX "(c)"  

#define HELPER_MAX_LOADSTRING 100