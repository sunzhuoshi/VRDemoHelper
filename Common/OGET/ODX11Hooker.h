#pragma once
#include "OGraphicsAPICommonHooker.h"
#include <list>

class ODX11Hooker : public OGraphicsAPICommonHooker
{
public:
    ODX11Hooker();
    ~ODX11Hooker();
    virtual OIGraphicsAPIHooker::HookerType getType() { return OIGraphicsAPIHooker::HT_D3D11; }
    virtual void hookWithWindow(HWND wnd);
    virtual void unhook();
private:
    std::list<void *> m_hookTargets;
};

