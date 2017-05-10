#pragma once
class OInlineHookUtil
{
public:
    static bool init();
    static void unint();
    static bool hook(void* targetFunc, void* detourFunc, void**originalFunc);
};
