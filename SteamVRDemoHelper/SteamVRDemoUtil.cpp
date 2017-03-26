#include "stdafx.h"
#include "SteamVRDemoUtil.h"

#include <sstream>

namespace steam_vr_demo_helper {

	std::string getCurrentExePath()
	{
		char buf[MAX_PATH] = "", *tmp;
		if (GetModuleFileName(NULL, buf, MAX_PATH))
		{
			tmp = _tcsrchr(buf, _T('\\'));
			if (tmp) {
				*(tmp + 1) = _T('\0');
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

} // namespace