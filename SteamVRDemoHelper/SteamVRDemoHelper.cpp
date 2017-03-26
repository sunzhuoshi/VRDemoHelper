// SteamVRDemoHelper.cpp : 定义应用程序的入口点。
//

// NOTE: DO NOT add anything except comments before "stdafx.h", it will be omitted without any warning...
#include "stdafx.h"

// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

#include "SteamVRDemoHelper.h"
#include "SteamVRDemoLogServer.h"
#include "SteamVRDemoUtil.h"
#include "XGetopt.h"
#include "argcargv.h"

#include <windows.h>
#include <shellapi.h>
#include <CommCtrl.h>

#include <log4cplus/log4cplus.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/initializer.h>
#include <log4cplus/socketappender.h>

#define MAX_LOADSTRING 100
#define HOOK_DLL_32_STRING "SteamVRDemoCore.dll"
//#define HOOK_DLL_64_STRING ""
#define LOG_PROPERTY_FILE "log4cplus.props"
#define RULE_CONFIG_FILE "rule_config.ini"

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
CHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
CHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
struct Config 								   // Parameters configured by command line arguments
{
	USHORT usPort;			// log server port
	BOOL bTrace;			// only start log server when trace is on
	Config():
		usPort(8888),
		bTrace(FALSE)
	{
	}
} config;
log4cplus::Logger logger;

HINSTANCE hInstHookDll64;
HOOKPROC hkprcWndMsgProc64;
HHOOK hhookWndMsgProc64;
typedef BOOL (*FnInit)(const CHAR *, BOOL);

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

// Use a guid to uniquely identify our icon
#ifdef _DEBUG
class __declspec(uuid("BC719626-7CD0-4FF2-B9B4-6D821515C9D6")) HelperIcon;
#else 
class __declspec(uuid("BC719626-7CD0-4FF2-B9B4-6D821515C9D7")) HelperIcon;
#endif 

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL AddNotificationIcon(HWND hwnd);
BOOL DeleteNotificationIcon();
void ShowContextMenu(HWND hwnd, POINT pt);
BOOL InitHookModule();
void UninitHookModule();
BOOL ParseCommandLineArguments();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// init log first in main
	log4cplus::Initializer initializer;
	log4cplus::PropertyConfigurator::doConfigure(LOG_PROPERTY_FILE);
	logger = log4cplus::Logger::getInstance("CLIENT");
	LOG4CPLUS_INFO(logger, "SteamVRDemoHelper is starting");

	if (!ParseCommandLineArguments())
	{
		return FALSE;
	}	

	SteamVRDemoLogServer::SteamVRDemoLogServerPtr logServer;
	if (config.bTrace) 
	{
		logServer = new SteamVRDemoLogServer();
		if (!logServer->start(config.usPort))
		{
			return FALSE;
		}
	}

	if (!InitHookModule())
	{
		return FALSE;
	}

    // 初始化全局字符串
    LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringA(hInstance, IDC_STEAMVRDEMOHELPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STEAMVRDEMOHELPER));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	if (config.bTrace) {
		logServer->stop();
	}

	LOG4CPLUS_INFO(logger, "SteamVRDemoHelper exited");
    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCEA(IDI_STEAMVRDEMOHELPER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEA(IDC_STEAMVRDEMOHELPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCEA(IDI_NOTIFICATIONICON));

    return RegisterClassExA(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowA(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
			AddNotificationIcon(hWnd);
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		UninitHookModule();
		DeleteNotificationIcon();
        PostQuitMessage(0);
        break;
	case WMAPP_NOTIFYCALLBACK:
		switch (LOWORD(lParam))
		{
		case WM_CONTEXTMENU:
			{
				POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
				ShowContextMenu(hWnd, pt);
			}
			break;
		default:
			break;
		}
		break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


BOOL AddNotificationIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	// add the icon, setting the icon, tooltip, and callback message.
	// the icon will be identified with the GUID
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	nid.guidItem = __uuidof(HelperIcon);
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	LoadIconMetric(hInst, MAKEINTRESOURCEW(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
	LoadString(hInst, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
	Shell_NotifyIcon(NIM_ADD, &nid);

	// NOTIFYICON_VERSION_4 is prefered
	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL DeleteNotificationIcon()
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.uFlags = NIF_GUID;
	nid.guidItem = __uuidof(HelperIcon);
	return Shell_NotifyIcon(NIM_DELETE, &nid);
} 

void ShowContextMenu(HWND hwnd, POINT pt)
{
	HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
	if (hMenu)
	{
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		if (hSubMenu)
		{
			// our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
			SetForegroundWindow(hwnd);

			// respect menu drop alignment
			UINT uFlags = TPM_RIGHTBUTTON;
			if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			{
				uFlags |= TPM_RIGHTALIGN;
			}
			else
			{
				uFlags |= TPM_LEFTALIGN;
			}

			TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
		}
		DestroyMenu(hMenu);
	}
}

BOOL InitHookModule()
{
	BOOL result = TRUE;

	hInstHookDll64 = LoadLibrary(steam_vr_demo_helper::getFileFullPath(HOOK_DLL_32_STRING).c_str());
	hkprcWndMsgProc64 = (HOOKPROC)GetProcAddress(hInstHookDll64, "fnWndMsgProc");
	FnInit fnInit = (FnInit)GetProcAddress(hInstHookDll64, "fnInit");
	if (fnInit) {
		if (fnInit(steam_vr_demo_helper::getFileFullPath(RULE_CONFIG_FILE).c_str(), config.bTrace)) {
			hhookWndMsgProc64 = SetWindowsHookEx(
				WH_CBT,
				hkprcWndMsgProc64,
				hInstHookDll64,
				0
			);
			if (!hhookWndMsgProc64)
			{
				result = FALSE;
			}
		}
		else {
			// TODO: show init error message box 
		}
	}
	else {
		// show error message box
	}
	if (!result)
	{
		FreeLibrary(hInstHookDll64);
		hInstHookDll64 = NULL;
		hkprcWndMsgProc64 = NULL;
	}
	return result;
}

void UninitHookModule()
{
	if (hhookWndMsgProc64)
	{
		UnhookWindowsHookEx(hhookWndMsgProc64);
		hhookWndMsgProc64 = NULL;
		FreeLibrary(hInstHookDll64);
		hInstHookDll64 = NULL;
	}
}

BOOL ParseCommandLineArguments()
{
	BOOL result = TRUE;
	char *cmdLine = GetCommandLineA();

	// eat the first token(exe path)
	int count = 0;
	while (cmdLine) 
	{
		if ('"' == *cmdLine)
		{
			count++;
		}
		cmdLine++;
		if (2 == count)
		{
			break;
		}
	}

	char c;
	while ((c = getopt(_ConvertCommandLineToArgcArgv(cmdLine), _ppszArgv, "tp:")) != EOF)
	{
		switch (c)
		{
		case 't':
			LOG4CPLUS_DEBUG(logger, "option t used" << std::endl);
			config.bTrace = TRUE;
			break;
		case 'p':
			LOG4CPLUS_DEBUG(logger, "option p: " << optarg << std::endl);
			config.usPort = atoi(optarg);
			break;
		case '?':
		default:
			{
			std::ostringstream msg;
			msg << "Unknown arguments:\n" << cmdLine;
			MessageBox(NULL, msg.str().c_str(), "Error", MB_OK);
			result = FALSE;
			break;
			}
		}
	}
	return result;
}
