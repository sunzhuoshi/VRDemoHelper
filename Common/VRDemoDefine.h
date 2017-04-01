#pragma once
#include <sstream>
#include <WinUser.h>
#include "util\l4util.h"

#define VR_DEMO_LOGGER_CLIENT "CLIENT"
#define VR_DEMO_LOGGER_SERVER "SERVER"


#define VR_DEMO_SUPPRESS_DOWHILE_WARNING()      \
    __pragma (warning (push))                   \
    __pragma (warning (disable:4127))

#define VR_DEMO_RESTORE_DOWHILE_WARNING()       \
    __pragma (warning (pop))


#define VR_DEMO_ALERT(captionStringId, text)                                                        \
    VR_DEMO_SUPPRESS_DOWHILE_WARNING()                                                              \
    do {                                                                                            \
        std::ostringstream msg;                                                                     \
        msg << text;                                                                                \
        MessageBoxA(NULL, msg.str().c_str(), l4util::loadString(captionStringId).c_str(), MB_OK);   \
    } while (false)                                                                                 \
    VR_DEMO_RESTORE_DOWHILE_WARNING()

