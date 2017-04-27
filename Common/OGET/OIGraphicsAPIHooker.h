#pragma once
#include <string>

class OIGraphicsAPIHooker {
public:
    enum HookerType {
        HT_D3D11,
        HT_D3D12,
        HT_D3D10,
        HT_D3D9,
        HT_OPENGL,
        HT_VULKAN
    };
    virtual ~OIGraphicsAPIHooker() {};
    virtual HookerType getType() = 0;
    virtual void hook() = 0;
    virtual void unhook() = 0;
};