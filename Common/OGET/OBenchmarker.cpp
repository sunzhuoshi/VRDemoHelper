#include "stdafx.h"

#include "OBenchmarker.h"

#include <time.h>
#include <iomanip>
#include <fstream>
#include <direct.h>

#include "util\l4util.h"

extern char g_rootPath[MAX_PATH];

const std::string OBenchmarker::DIR_BENCHMARK = "benchmark";

OBenchmarker::OBenchmarker()
{
}


OBenchmarker::~OBenchmarker()
{
}

bool OBenchmarker::init(VRDemoArbiter::Toggles& toggles)
{
    bool ret = (0 != QueryPerformanceFrequency(&m_frequency));
    
    if (ret) {
        m_toggles = &toggles;
        srand((unsigned int)time(NULL));
        m_inited = true;
    }
    return m_inited;
}

void OBenchmarker::newFrame()
{
    if (m_inited && m_benchmarking) {
        LARGE_INTEGER now;
        if (0 != QueryPerformanceCounter(&now)) {
            m_frameCounters.push_back(now);
        }
    }
}

bool OBenchmarker::toggle()
{
    if (m_inited) {
        if (m_benchmarking) {
            endBenchmark();
            m_benchmarking = false;
            *m_toggles = m_oldToggles;
        }
        else {
            m_oldToggles = *m_toggles;
            m_toggles->m_showFPS = false;
            m_benchmarking = true;
            beginBenchmark();
        }
    }
    return m_benchmarking;
}

void OBenchmarker::beginBenchmark()
{
    if (m_inited) {
        m_frameCounters.clear();
        time(&m_benchmarkTime);
    }
}

void OBenchmarker::endBenchmark()
{
    if (m_inited) {
        Json::Value result = generateResult();
        if (isResultValid(result)) {
            saveResultToFile(result);
        }
    }
}

bool OBenchmarker::createBenchmarkDir()
{
    std::string benchmarkDirFullPath = std::string(g_rootPath) + DIR_BENCHMARK;

    // remove file with same name as benchmark folder
    remove(benchmarkDirFullPath.c_str());

    return 0 == _mkdir(benchmarkDirFullPath.c_str()) ? true : EEXIST == errno;
}

bool OBenchmarker::saveResultToFile(Json::Value& result)
{
    if (createBenchmarkDir()) {
        std::ofstream file(getFileFullPath());

        if (file.good()) {
            Json::StreamWriterBuilder builder;
            builder["precision"] = 3;

            Json::StreamWriter* writer = builder.newStreamWriter();
            writer->write(result, &file);
            return true;
        }
    }
    return false;
}


bool OBenchmarker::isResultValid(Json::Value& result)
{
    return result["minFPS"].asInt() <= result["maxFPS"].asInt();
}


Json::Value OBenchmarker::generateResult()
{
    Json::Value resultValue, tagsValue(Json::arrayValue);
    
    resultValue["environment"] = getEnvironment();
    resultValue["target"] = getTarget();
    resultValue["time"] = getTime();
    resultValue["label"] = "";
    resultValue["tags"] = tagsValue;
    setFPSData(resultValue);
    setFrameData(resultValue);
    return resultValue;
}

Json::Value OBenchmarker::getEnvironment()
{
    Json::Value rs;

    // TODO: finish it
    rs["cpu"] = "Intel(R) Core(TM) i7-4720HQ CPU @2.6GHz(8 CPUs), ~2.6Hz";
    rs["os"] = "Windows 8.1 中文版64位(6.3, 版本9600)";
    rs["memory"] = "8192MB";
    rs["graphics"] = "nVidia GeForce GTX 960M";
    return rs;
}

std::string OBenchmarker::getTarget()
{
    std::string target;
    std::string processName = l4util::getCurrentProcessName();
    size_t index = processName.rfind('.');

    if (index == std::string::npos) {
        target = processName;
    }
    else {
        target = processName.substr(0, index);
    }
    return target;
}

std::string OBenchmarker::getTime()
{
    struct tm timeInfo;
    char buf[128] = { 0 };
  
    if (0 == localtime_s(&timeInfo, &m_benchmarkTime)) {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H-%M-%S", &timeInfo);
    }
    return buf;
}

std::string OBenchmarker::getResultKey()
{
    std::stringstream buf;
    buf << getTarget() << " " << getTime() << '-' << std::setw(3) << rand() % 1000;
    return buf.str();
}

std::string OBenchmarker::getFileName()
{
    return getResultKey() + ".json";
}

std::string OBenchmarker::getFileFullPath()
{
    std::ostringstream buf;
    buf << g_rootPath << DIR_BENCHMARK << '\\' << getFileName();
    return buf.str();
}


void OBenchmarker::setFrameData(Json::Value& result)
{
    std::list<double> frameTimeList;
    double duration = 0.0;
    std::ostringstream strBuf;

    result["frameCount"] = m_frameCounters.size();

    if (m_frameCounters.size()) {
        duration = 1000.0 * (m_frameCounters.back().QuadPart - m_frameCounters.front().QuadPart) / m_frequency.QuadPart;
    }
    strBuf.str("");
    strBuf.clear();
    strBuf << long long(duration + 0.5) << "ms";
    result["duration"] = strBuf.str();

    LARGE_INTEGER lastFrameTime = { 0 };
    for (const auto& it : m_frameCounters) {
        if (0 != lastFrameTime.QuadPart) {
            frameTimeList.push_back(1000.0 * (it.QuadPart - lastFrameTime.QuadPart) / m_frequency.QuadPart);
        }
        lastFrameTime = it;
    }

    Json::Value frameTimeListValue;
    int index = 0;
    for (auto& it : frameTimeList) {
        frameTimeListValue[index++] = it;
    }
    result["frameTimeList"] = frameTimeListValue;
}

void OBenchmarker::setFPSData(Json::Value& result)
{
    size_t fps = 0;
    size_t minFPS = -1, maxFPS = 0;
    double averageFPS = 0.0;
    std::list<size_t> fpsList;

    if (m_frameCounters.size()) {
        LARGE_INTEGER endCounter;
        endCounter.QuadPart = m_frameCounters.front().QuadPart + m_frequency.QuadPart;

        for (auto& it : m_frameCounters) {
            if (it.QuadPart <= endCounter.QuadPart) {
                fps++;
            }
            else {
                if (fps > maxFPS) {
                    maxFPS = fps;
                }
                if (fps < minFPS) {
                    minFPS = fps;
                }
                fpsList.push_back(fps);
                fps = 1;
                endCounter.QuadPart += m_frequency.QuadPart;
            }
        }
        // the last second
        if ((m_frameCounters.back().QuadPart - m_frameCounters.front().QuadPart) % m_frequency.QuadPart > 0.999 * m_frequency.QuadPart) {
            if (fps > maxFPS) {
                maxFPS = fps;
            }
            if (fps < minFPS) {
                minFPS = fps;
            }
            fpsList.push_back(fps);
        }
        averageFPS = (double)m_frameCounters.size() * m_frequency.QuadPart / (m_frameCounters.back().QuadPart - m_frameCounters.front().QuadPart);
    }
    result["maxFPS"] = maxFPS;
    result["minFPS"] = minFPS;
    result["averageFPS"] = averageFPS;
    // fps list
    Json::Value fpsListValue;
    int index = 0;
    for (auto& it : fpsList) {
        fpsListValue[index++] = it;
    }
    result["fpsList"] = fpsListValue;
}

