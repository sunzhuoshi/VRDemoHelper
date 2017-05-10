#pragma once
class OTFPSDrawUtil
{
public:
    static bool init();
    static void uninit();
    static void drawFPS(unsigned int FPS);
    OTFPSDrawUtil();
    ~OTFPSDrawUtil();
};

