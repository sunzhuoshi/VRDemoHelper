#pragma once

#include <map>

#include "OIGraphicsAPIHooker.h"

class OGraphicsAPIHookerFactory
{
public:
    typedef std::map<std::string, int> HookerModuleTypeMap;
public:
    static OIGraphicsAPIHooker *createHooker();
    OGraphicsAPIHookerFactory();
    ~OGraphicsAPIHookerFactory();
private:
    static const HookerModuleTypeMap m_hookerModuleTypeMap;
};
