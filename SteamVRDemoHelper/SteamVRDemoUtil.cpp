#include "stdafx.h"
#include "SteamVRDemoUtil.h"

#include <sstream>

namespace steam_vr_demo_helper {

	std::string getCurrentExePath()
	{
		char buf[MAX_PATH] = "", *tmp;
		if (GetModuleFileNameA(NULL, buf, MAX_PATH))
		{
			tmp = strrchr(buf, '\\');
			if (tmp) {
				*(tmp + 1) = '\0';
			}
		}
		return std::string(buf);
	}

	std::string getHookDllPath(const std::string &dllName)
	{
		std::ostringstream result;
		result << getCurrentExePath().c_str() << "\\" << dllName.c_str();
		return result.str();
	}

	std::string getCurrentProcessName()
	{
		char buf[MAX_PATH] = "", *tmp;
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
} // namespace