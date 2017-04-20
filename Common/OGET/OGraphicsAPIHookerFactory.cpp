#include "stdafx.h"
#include "OGraphicsAPIHookerFactory.h"

#include "ODX11Hooker.h"

const OGraphicsAPIHookerFactory::HookerModuleTypeMap OGraphicsAPIHookerFactory::m_hookerModuleTypeMap =
{
    { "d3d11.dll", OIGraphicsAPIHooker::HT_D3D11 }
};

OIGraphicsAPIHooker* OGraphicsAPIHookerFactory::createHooker()
{
    OIGraphicsAPIHooker *hooker = nullptr;
    int hookerType = -1;

    for (const auto& it : m_hookerModuleTypeMap) {
        if (GetModuleHandleA(it.first.c_str())) {
            hookerType = it.second;
            break;
        }
    }

    switch (hookerType) {
    case OIGraphicsAPIHooker::HT_D3D11:
        hooker = new (std::nothrow) ODX11Hooker();
        break;
    case OIGraphicsAPIHooker::HT_D3D9:
    case OIGraphicsAPIHooker::HT_D3D10:
    case OIGraphicsAPIHooker::HT_D3D12:
    case OIGraphicsAPIHooker::HT_OPENGL:
    case OIGraphicsAPIHooker::HT_VULKAN:
    default:
        break;
    }
    return hooker;
}


OGraphicsAPIHookerFactory::OGraphicsAPIHookerFactory()
{
}


OGraphicsAPIHookerFactory::~OGraphicsAPIHookerFactory()
{
}
