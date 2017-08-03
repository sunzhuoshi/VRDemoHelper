#pragma once

#ifndef _WIN64

#include "util\l4Singleton.hpp"

class VRDemoConfigurator;

class VRDemoAutoRunner : public L4Singleton<VRDemoAutoRunner>
{
public:
    VRDemoAutoRunner();
    ~VRDemoAutoRunner();
    void setup(VRDemoConfigurator& configurator);
};

#endif // _WIN64