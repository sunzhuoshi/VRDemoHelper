#pragma once
#include <list>
#include "util\L4Singleton.hpp"

#include "VRDemoArbiter.h"

#include "json\json.h"

class OBenchmarker : public L4Singleton<OBenchmarker>
{
public:
    OBenchmarker();
    ~OBenchmarker();
    bool init(VRDemoArbiter::Toggles& toggles);
    void newFrame();
    bool toggle();
    static const std::string DIR_BENCHMARK;
private:
    void beginBenchmark();
    void endBenchmark();
    bool createBenchmarkDir();
    bool saveResultToFile(Json::Value& result);
    bool isResultValid(Json::Value& result);
    Json::Value generateResult();
    Json::Value getEnvironment();
    std::string getTarget();
    std::string getTime();
    std::string getFileName();
    std::string getFileFullPath();
    void setFrameData(Json::Value& result);
    void setFPSData(Json::Value& result);
    VRDemoArbiter::Toggles m_oldToggles;
    VRDemoArbiter::Toggles* m_toggles = nullptr;
    LARGE_INTEGER m_frequency = { 0, 0 };
    std::list<LARGE_INTEGER> m_frameCounters;
    bool m_benchmarking = false;
    time_t m_benchmarkTime = 0;
    bool m_inited = false;
};

