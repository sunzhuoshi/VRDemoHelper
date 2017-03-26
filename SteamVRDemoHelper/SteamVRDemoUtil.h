#pragma once
#include <string>

namespace steam_vr_demo_helper {
	std::string getCurrentExePath();
	std::string getHookDllPath(const std::string &dllName);
}