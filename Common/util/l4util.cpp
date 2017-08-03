#include "stdafx.h"

#include "l4util.h"
#include <sstream> 
#include <minwindef.h>
#include <Windows.h>
#include <Psapi.h>

namespace l4util {
    // ends with '\\'
	std::string getCurrentExePath()
	{
		char buf[MAX_PATH] = "", *tmp = nullptr;
		if (GetModuleFileNameA(NULL, buf, MAX_PATH))
		{
			tmp = strrchr(buf, '\\');
			if (tmp && tmp < buf + sizeof(buf) - 1) {
				tmp[1] = '\0';
			}
		}
		return std::string(buf);
	}

	std::string getFileFullPath(const std::string &dllFileName)
	{
        return getCurrentExePath() + dllFileName;
	}

	std::string getCurrentProcessName()
	{
		char buf[MAX_PATH] = "", *tmp = NULL;
		std::string result;

		if (GetModuleFileNameA(NULL, buf, MAX_PATH))
		{
			tmp = strrchr(buf, '\\');
			if (tmp) {
				tmp++;
			}
		}
		if (tmp) {
			result = tmp;
		}
		return result;
	}

    std::string getProcessNameWithWindow(HWND wnd)
    {
        DWORD processId;

        GetWindowThreadProcessId(wnd, &processId);
        return getProcessNameWithProcessId(processId);
    }

    std::string getProcessNameWithProcessId(DWORD processId)
    {
        char buf[MAX_PATH] = "", *p = nullptr;

        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);

        if (processHandle) {
            GetProcessImageFileNameA(processHandle, buf, sizeof(buf));
            p = strrchr(buf, '\\');
            if (p) {
                p++;
            }
            CloseHandle(processHandle);
        }
        else {
            int i = 0;
            i++;
        }
        if (p) {
            return p;
        }
        else {
            return "";
        }
    }

	std::string loadString(UINT id) 
	{
		std::string result;
		char buf[MAX_PATH];	// just be lazy to use MAX_PATH
		HMODULE instance = GetModuleHandle(NULL);
		
		if (instance) {
			if (0 < LoadStringA(instance, id, buf, MAX_PATH)) {
				result = std::string(buf);
			}
		}
		return result;
	}

    bool isWin64()
    {
#if defined(_WIN64)
        return true;  // 64-bit programs run only on Win64
#else
        // 32-bit programs run on both 32-bit and 64-bit Windows
        // so must sniff
        BOOL f64 = FALSE;
        return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#endif
    }
}