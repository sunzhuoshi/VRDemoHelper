// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <Windows.h>
#include <d3d11.h>

#include "FW1FontWrapper\FW1FontWrapper.h"
#include "MiniHook\MinHook.h"
#include <list>

inline MH_STATUS MH_CreateHookAndEnable(LPVOID targetFunc, LPVOID detourFunc, LPVOID *originalFunc)
{  
    MH_STATUS ret = MH_CreateHook(targetFunc, detourFunc, originalFunc);
    if (MH_OK == ret) {
        ret = MH_EnableHook(targetFunc);
    }
    return ret;
}

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;

DWORD_PTR* pSwapChainVtable = NULL;

D3D11PresentHook originalD3D11Present = NULL;
LPVOID targetD3D11Present = NULL;


IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;


LARGE_INTEGER frequencey;

bool firstTime = true;

bool CalculateFPS(std::list<LARGE_INTEGER> &frameDataList, unsigned int& fps)
{
    if (!frameDataList.empty() && frameDataList.back().QuadPart - frameDataList.front().QuadPart > frequencey.QuadPart) {
        unsigned int count = 0;
        std::list<LARGE_INTEGER>::const_reverse_iterator it = frameDataList.rbegin();
        while (frameDataList.back().QuadPart - it->QuadPart < frequencey.QuadPart && it != frameDataList.rend() && it != frameDataList.crend()) {
            it++;
            count++;
        }
        if (it != frameDataList.crend()) {
            fps = static_cast<int>(double(count)  * frequencey.QuadPart / (frameDataList.back().QuadPart - it->QuadPart) + 0.5);
            return true;
        }
    }
    return false;
}

void DrawFPS(unsigned int fps)
{
    WCHAR fpsText[16];
    swprintf_s(fpsText, sizeof(fpsText) / sizeof(fpsText[0]), L"%d", fps);
    fpsText[sizeof(fpsText)/sizeof(fpsText[0]) - 1] = '\0';
    pFontWrapper->DrawString(pContext, fpsText, 20.0f, 16.0f, 16.0f, 0xFF00FF00, FW1_RESTORESTATE);
}

HRESULT __stdcall DetourD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static std::list<LARGE_INTEGER> frameDataList;

    LARGE_INTEGER now;
    unsigned int  fps = 0;


    if (firstTime)
    {
        pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
        pDevice->GetImmediateContext(&pContext);

        FW1CreateFactory(FW1_VERSION, &pFW1Factory);
        pFW1Factory->CreateFontWrapper(pDevice, L"Arial", &pFontWrapper);

        pFW1Factory->Release();

        firstTime = false;

        QueryPerformanceFrequency(&frequencey);
    }

    QueryPerformanceCounter(&now);

    frameDataList.push_back(now);

    if (CalculateFPS(frameDataList, fps)) {
        DrawFPS(fps);
    }

    return originalD3D11Present(pSwapChain, SyncInterval, Flags);
}

DWORD __stdcall InitializeHook(LPVOID)
{
    HWND hWnd = GetForegroundWindow();
    IDXGISwapChain* pSwapChain;

    if (!GetModuleHandle("d3d11.dll")) {
        return FALSE;
    }

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
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
        , D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext)))
    {
        return FALSE;
    }

    pSwapChainVtable = (DWORD_PTR*)pSwapChain;
    pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

    targetD3D11Present = (LPVOID)pSwapChainVtable[8];

    MH_CreateHookAndEnable(targetD3D11Present, DetourD3D11Present, (LPVOID *)&originalD3D11Present);

    pDevice->Release();
    pContext->Release();
    pSwapChain->Release();

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
        MH_Uninitialize();
        if (pFontWrapper)
        {
            pFontWrapper->Release();
        }
		break;
	}
	return TRUE;
}

