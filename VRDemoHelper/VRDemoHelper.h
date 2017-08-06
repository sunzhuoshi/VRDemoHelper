#pragma once

#include "resource.h"

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

#define HELPER_SECTION_NAME "VRDemoHelper"

// to find message only window of child process, we have to use a differient window title
// or it will return parent process window
#define HELPER_WINDOW_TITLE_CHILD_PROCESS_POSTFIX "(c)"  

#define HELPER_MAX_LOADSTRING 100

enum ExitCode {
    E_NOT_ABLE_TO_RUN = 40001,
    E_INIT_FAILURE_CONFIGURATOR,
    E_INIT_FAILURE_CORE,
    E_INIT_FAILURE_POLLER,
    E_INIT_FAILURE_INSTANCE,
    E_INIT_FAILURE_KEEPER
};

void AyncCloseApplication();