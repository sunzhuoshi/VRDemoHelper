#pragma once
class VRDemoHotKeyManager
{
public:
    VRDemoHotKeyManager();
    ~VRDemoHotKeyManager();
    void configurate(HWND wnd);
    static const std::string HOT_KEY_PAUSE;
};

