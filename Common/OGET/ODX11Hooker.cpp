#include "stdafx.h"
#include "ODX11Hooker.h"

#include <d3d11.h>

#include "OInlineHookUtil.h"
#include "OFPSCalculator.h"
#include "FW1FontWrapper\FW1FontWrapper.h"
#include "VRDemoArbiter.h"
#include "OGET\OBenchmarker.h"

// TODO: remove dependency to VRDemoArbiter
extern VRDemoArbiter::Toggles g_toggles;

typedef HRESULT(__stdcall *D3D11SwapChainPresentFunc) (IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall *D3D11SwapChainResizeBuffersFunc) (IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
typedef HRESULT(__stdcall *D3D11SwapChainSetFullscreenStateFunc) (IDXGISwapChain*, BOOL, IDXGIOutput *);
D3D11SwapChainPresentFunc g_originalD3D11SwapChainPresent = NULL;
D3D11SwapChainResizeBuffersFunc g_originalD3D11SwapChainResizeBuffers = NULL;
D3D11SwapChainSetFullscreenStateFunc g_originalD3D11SwapChainSetFullscreenState = NULL;

ID3D11Device *g_device = NULL;
ID3D11DeviceContext *g_context = NULL;
ID3D11RenderTargetView *g_backBufferRenderTargetView = NULL;

bool g_needUpdateBackBufferRenderTargetView = false;

IFW1FontWrapper *g_fontWrapper = NULL;

void DrawFPS(unsigned int FPS)
{
    WCHAR fpsText[16];
    swprintf_s(fpsText, sizeof(fpsText) / sizeof(fpsText[0]), L"%d", FPS);
    fpsText[sizeof(fpsText) / sizeof(fpsText[0]) - 1] = '\0';
    g_fontWrapper->DrawString(g_context, fpsText, 20.0f, 16.0f, 16.0f, 0xFF00FF00, FW1_RESTORESTATE);
}

void UpdateBackBufferRenderTargetView(IDXGISwapChain* swapChain) {
    ID3D11Texture2D* backBuffer = nullptr;

    if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))) {
        if (g_backBufferRenderTargetView) {
            g_backBufferRenderTargetView->Release();
            g_backBufferRenderTargetView = nullptr;
        }
        g_device->CreateRenderTargetView(backBuffer, nullptr, &g_backBufferRenderTargetView);
        backBuffer->Release();
        g_needUpdateBackBufferRenderTargetView = false;
    }
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
    if (g_backBufferRenderTargetView) {
        g_backBufferRenderTargetView->Release();
        g_backBufferRenderTargetView = nullptr;
    }

    HRESULT ret = g_originalD3D11SwapChainResizeBuffers(
        pSwapChain,
        BufferCount,
        Width,
        Height,
        NewFormat,
        SwapChainFlags
    );
    if (SUCCEEDED(ret)) {
        g_needUpdateBackBufferRenderTargetView = true;
    }
    return ret;
}

HRESULT __stdcall DetourD3D11Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
    static bool inited = false;
    static bool fpsInitOK = false;
    static OFPSCalculator<10000> fpsCalculator;

    if (!inited) {
        swapChain->GetDevice(__uuidof(g_device), (void**)&g_device);
        g_device->GetImmediateContext(&g_context);

        UpdateBackBufferRenderTargetView(swapChain);

        IFW1Factory *pFW1Factory = NULL;
        FW1CreateFactory(FW1_VERSION, &pFW1Factory);
        pFW1Factory->CreateFontWrapper(g_device, L"Arial", &g_fontWrapper);
        pFW1Factory->Release();

        fpsInitOK = fpsCalculator.init();
        inited = true;
    }
    if (!g_toggles.m_pause) {
        OBenchmarker::getInstance().newFrame();
        if (fpsInitOK && g_toggles.m_showFPS) {
            unsigned int  fps = 0;
            if (g_needUpdateBackBufferRenderTargetView) {
                UpdateBackBufferRenderTargetView(swapChain);
            }
            g_context->OMSetRenderTargets(1, &g_backBufferRenderTargetView, nullptr);

            if (fpsCalculator.presentNewFrameAndGetFPS(fps)) {
                DrawFPS(fps);
            }
        }
    }
    return g_originalD3D11SwapChainPresent(swapChain, syncInterval, flags);
}

HRESULT DetourD3D11SetFullscreenState(IDXGISwapChain* swapChain, BOOL fullscreen, IDXGIOutput *target) {
    HRESULT ret = g_originalD3D11SwapChainSetFullscreenState(swapChain, fullscreen, target);
    g_needUpdateBackBufferRenderTargetView = SUCCEEDED(ret);
    return ret;
}

ODX11Hooker::ODX11Hooker()
{
}


ODX11Hooker::~ODX11Hooker()
{
    if (g_fontWrapper) {
        g_fontWrapper->Release();
        g_fontWrapper = nullptr;
    }
}

void ODX11Hooker::hookWithWindow(HWND wnd)
{
    IDXGISwapChain* swapChain = NULL;
    ID3D11Device *device = NULL;
    ID3D11DeviceContext* context = NULL;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = wnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, 
        &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &context))) {
        DWORD_PTR** tmp = reinterpret_cast<DWORD_PTR **>(swapChain);
        DWORD_PTR* swapChainVirtualTable = tmp[0];

        // hook swap chain "Present" method to draw FPS
        OInlineHookUtil::hook((void *)swapChainVirtualTable[8], DetourD3D11Present, (void **)&g_originalD3D11SwapChainPresent);
        // hook swap chain "ResizeBuffers" method to update back buffer render target view(Unreal 4 requires this, Unity, WoW don't)
        OInlineHookUtil::hook((void *)swapChainVirtualTable[13], DetourD3D11ResizeBuffers, (void **)&g_originalD3D11SwapChainResizeBuffers);
        // hook swap chain "SetFullScreenState" method, for some Unity games don't call ResizeBuffers when toggling to fullscreen mode
        OInlineHookUtil::hook((void *)swapChainVirtualTable[10], DetourD3D11SetFullscreenState, (void **)&g_originalD3D11SwapChainSetFullscreenState);

        device->Release();
        context->Release();
        swapChain->Release();
    }
}

void ODX11Hooker::unhook()
{

}
