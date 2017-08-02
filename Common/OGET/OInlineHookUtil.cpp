#include "stdafx.h"

#include "OInlineHookUtil.h"

#include "MiniHook\MinHook.h"

bool OInlineHookUtil::init()
{
#if WITH_FPS
    return MH_OK == MH_Initialize();
#else
    return false;
#endif // WITH_FPS
}

void OInlineHookUtil::unint()
{
#if WITH_FPS
    MH_Uninitialize();
#endif // WITH_FPS
}

bool OInlineHookUtil::hook(void* targetFunc, void* detourFunc, void**originalFunc)
{
#if WITH_FPS
    MH_STATUS ret = MH_CreateHook(targetFunc, detourFunc, originalFunc);
    if (MH_OK == ret) {
        ret = MH_EnableHook(targetFunc);
    }
    return MH_OK == ret;
#else
    return false;
#endif // WITH_FPS
}

bool OInlineHookUtil::unhook(void *targetFunc)
{
    if (!targetFunc) {
        return false;
    }
#if WITH_FPS
    return MH_OK == MH_RemoveHook(targetFunc);
#else
    return false;
#endif // WITH_FPS
}