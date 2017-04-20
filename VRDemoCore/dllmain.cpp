// dllmain.cpp : 定义 DLL 应用程序的入口点。

// TODO: fix the crash issue if it quits first when running together with Fraps
#include "stdafx.h"

#include <Windows.h>
#include <d3d11.h>

#include "FW1FontWrapper\FW1FontWrapper.h"
#include "MiniHook\MinHook.h"
#include <list>
#include <mutex>

#include "VRDemoArbiter.h"

extern VRDemoArbiter::Toggles toggles;

#include <d3d11shader.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

// TODO: minimize overhead as small as possible
template<unsigned int BUFFER_SIZE>
class FPSCalculator {
public:
    bool init() {
        return 0 != QueryPerformanceFrequency(&m_frequency);
    }
    bool presentNewFrameAndGetFPS(unsigned int &fps) {
        LARGE_INTEGER now;
        bool calculateFPS = false;
        if (QueryPerformanceCounter(&now)) {
            pushBack(now);
            // frames span over 1s 
            if (front().QuadPart - cursor().QuadPart >= m_frequency.QuadPart) {
                LARGE_INTEGER nextFrameData;
                // move the cursor to the newest one in 1s
                while (nextCursor(nextFrameData)) {
                    if (front().QuadPart - nextFrameData.QuadPart >= m_frequency.QuadPart) {
                        stepCursor();
                    }
                    else {
                        break;
                    }
                }
                calculateFPS = true;
            }
            // less than 1s, but buff is full
            else if (full()) {
                calculateFPS = true;
            }
        }
        if (calculateFPS) {
            fps = static_cast<unsigned int>(double((m_begin - m_cursor + BUFFER_SIZE) % BUFFER_SIZE + 1) * m_frequency.QuadPart / (front().QuadPart - cursor().QuadPart));
        }
        return calculateFPS;
    }
private:
    inline void pushBack(LARGE_INTEGER dataItem) {
        m_begin = (m_begin + 1) % BUFFER_SIZE;
        m_buff[m_begin] = dataItem;
        if (m_begin == m_end) {
            stepEnd();
        }
    }
    inline bool full() {
        return size() == BUFFER_SIZE - 1;
    }
    inline size_t size() {
        return (m_begin - m_end + BUFFER_SIZE) % BUFFER_SIZE;
    }
    inline LARGE_INTEGER cursor() {
        return m_buff[m_cursor];
    }
    inline LARGE_INTEGER front() {
        return m_buff[m_begin];
    }
    inline bool nextCursor(LARGE_INTEGER& dataItem) {
        unsigned int nextCursor = m_cursor + 1;
        if ((m_begin - nextCursor + BUFFER_SIZE) % BUFFER_SIZE) {
            dataItem = m_buff[nextCursor];
            return true;
        }
        return false;
    }
    inline void stepCursor() {
        if ((m_begin - m_cursor + BUFFER_SIZE) % BUFFER_SIZE) {
            m_cursor = (m_cursor + 1) % BUFFER_SIZE;
        }
    }
    inline void stepEnd() {
        m_end = (m_end + 1) % BUFFER_SIZE;
        if (m_end == m_cursor && (m_begin - m_cursor + BUFFER_SIZE) % BUFFER_SIZE) {
            m_cursor = (m_cursor + 1) % BUFFER_SIZE;
        }
    }
private:
    unsigned int m_begin = 0;               // the newest frame data index
    unsigned int m_end = 0;                 // the oldest frame data index - 1 (data assumed invalid)
    unsigned int m_cursor = 1;              // frame data between cursor and begin(span over 1s) will be used to calculate FPS  
    LARGE_INTEGER m_frequency = { 0 };
    LARGE_INTEGER m_buff[BUFFER_SIZE] = { 0 };
};

inline MH_STATUS MH_CreateHookAndEnable(LPVOID targetFunc, LPVOID detourFunc, LPVOID *originalFunc)
{  
    MH_STATUS ret = MH_CreateHook(targetFunc, detourFunc, originalFunc);
    if (MH_OK == ret) {
        ret = MH_EnableHook(targetFunc);
    }
    return ret;
}

typedef HRESULT(__stdcall *D3D11SwapChainPresentFunc) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall *D3D11SwapChainResizeBuffersFunc) (IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;
ID3D11RenderTargetView *pBackBufferRenderTargetView = NULL;

D3D11SwapChainPresentFunc originalD3D11SwapChainPresent = NULL;
LPVOID targetD3D11SwapChainPresent = NULL;
D3D11SwapChainResizeBuffersFunc originalD3D11SwapChainResizeBuffers = NULL;
LPVOID targetD3D11SwapChainResizeBuffers = NULL;

IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;
bool needUpdateBackBufferRenderTargetView = false;
// funny, lock is not needed for current tested games
//std::mutex presentMutex;

void UpdateBackBufferRenderTargetView(IDXGISwapChain* pSwapChain) {
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (SUCCEEDED(hr)) {
        if (pBackBufferRenderTargetView) {
            pBackBufferRenderTargetView->Release();
            pBackBufferRenderTargetView = nullptr;
        }
        pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pBackBufferRenderTargetView);
        pBackBuffer->Release();
        needUpdateBackBufferRenderTargetView = false;
    }
}

void DrawFPS(unsigned int fps)
{
    WCHAR fpsText[16];
    swprintf_s(fpsText, sizeof(fpsText) / sizeof(fpsText[0]), L"%d", fps);
    fpsText[sizeof(fpsText) / sizeof(fpsText[0]) - 1] = '\0';
    pFontWrapper->DrawString(pContext, fpsText, 20.0f, 16.0f, 16.0f, 0xFF00FF00, FW1_RESTORESTATE);
}

HRESULT __stdcall DetourD3D11ResizeBuffers(
    IDXGISwapChain* pSwapChain,
    UINT        BufferCount,
    UINT        Width,
    UINT        Height,
    DXGI_FORMAT NewFormat,
    UINT        SwapChainFlags
)
{
    //std::lock_guard<std::mutex> lock(presentMutex);
    if (pBackBufferRenderTargetView) {
        pBackBufferRenderTargetView->Release();
        pBackBufferRenderTargetView = nullptr;
    }

    HRESULT ret = originalD3D11SwapChainResizeBuffers(
        pSwapChain,
        BufferCount, 
        Width,
        Height,
        NewFormat,
        SwapChainFlags
    );
    if (SUCCEEDED(ret)) {
        needUpdateBackBufferRenderTargetView = true;
    }
    return ret;
}

HRESULT __stdcall DetourD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool firstTime = true;
    static bool fpsCalculatorInited = false;
    static FPSCalculator<10000> fpsCalculator;

    //std::lock_guard<std::mutex> lock(presentMutex);
    if (firstTime)
    {
        pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
        pDevice->GetImmediateContext(&pContext);

        UpdateBackBufferRenderTargetView(pSwapChain);

        FW1CreateFactory(FW1_VERSION, &pFW1Factory);
        pFW1Factory->CreateFontWrapper(pDevice, L"Arial", &pFontWrapper);
        pFW1Factory->Release();

        fpsCalculatorInited = fpsCalculator.init();

        firstTime = false;

    }
    if (needUpdateBackBufferRenderTargetView) {
        UpdateBackBufferRenderTargetView(pSwapChain);
    }
    pContext->OMSetRenderTargets(1, &pBackBufferRenderTargetView, nullptr);

    if (!toggles.m_pause && toggles.m_showFPS) {

        if (fpsCalculatorInited) {
            unsigned int  fps = 0;
            if (fpsCalculator.presentNewFrameAndGetFPS(fps)) {
                DrawFPS(fps);
            }                 
        }
    }
    return originalD3D11SwapChainPresent(pSwapChain, SyncInterval, Flags);
}

void HookD3D11(HWND hWnd)
{
    IDXGISwapChain* pSwapChain = nullptr;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    DWORD_PTR* pSwapChainVtable = NULL;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
        , D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext))) {
        VR_DEMO_ALERT_SS("Error", "Failed to create swap chain, error: " << GetLastError());
        return;
    }

    pSwapChainVtable = (DWORD_PTR*)pSwapChain;
    pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

    targetD3D11SwapChainPresent = (LPVOID)pSwapChainVtable[8];
    targetD3D11SwapChainResizeBuffers = (LPVOID)pSwapChainVtable[13];

    MH_CreateHookAndEnable(targetD3D11SwapChainResizeBuffers, DetourD3D11ResizeBuffers, (LPVOID *)&originalD3D11SwapChainResizeBuffers);
    MH_CreateHookAndEnable(targetD3D11SwapChainPresent, DetourD3D11Present, (LPVOID *)&originalD3D11SwapChainPresent);

    pDevice->Release();
    pContext->Release();
    pSwapChain->Release();
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    BOOL cont = TRUE;
    DWORD windowProcessID;

    GetWindowThreadProcessId(hWnd, &windowProcessID);
    if (GetCurrentProcessId() == windowProcessID) {
        cont = FALSE;
        HookD3D11(hWnd);
    }
    return cont;
}

DWORD __stdcall InitializeHook(LPVOID)
{
    if (!GetModuleHandleA("d3d11.dll")) {
        return FALSE;
    }
    HWND hWnd = GetForegroundWindow();

    if (hWnd) {
        HookD3D11(hWnd);
    }
    else {
        EnumWindows(&EnumWindowsProc, NULL);
    }
    return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        MH_Initialize();
        CreateThread(NULL, 0, InitializeHook, NULL, 0, NULL);
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        MH_DisableHook(targetD3D11SwapChainPresent);
        MH_Uninitialize();
        if (pFontWrapper)
        {
            pFontWrapper->Release();
        }
		break;
	}
	return TRUE;
}

