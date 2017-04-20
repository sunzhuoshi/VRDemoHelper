#include "stdafx.h"

#include "OInlineHookUtil.h"

#include "MiniHook\MinHook.h"

bool OInlineHookUtil::init()
{
    return MH_OK == MH_Initialize();
}

void OInlineHookUtil::unint()
{
    MH_Uninitialize();
}

bool OInlineHookUtil::hook(void* targetFunc, void* detourFunc, void**originalFunc)
{
    MH_STATUS ret = MH_CreateHook(targetFunc, detourFunc, originalFunc);
    if (MH_OK == ret) {
        ret = MH_EnableHook(targetFunc);
    }
    return MH_OK == ret;
}
