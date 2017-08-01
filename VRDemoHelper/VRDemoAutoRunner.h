#pragma once

#include "util\l4Singleton.hpp"

class VRDemoConfigurator;

class VRDemoAutoRunner : public L4Singleton<VRDemoAutoRunner>
{
public:
    VRDemoAutoRunner();
    ~VRDemoAutoRunner();
    void setup(VRDemoConfigurator& configurator);
};

