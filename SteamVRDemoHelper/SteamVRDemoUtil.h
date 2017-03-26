#pragma once
#include <string>

namespace steam_vr_demo_helper {
	// no last slash
	std::string getCurrentExePath();

	std::string getFileFullPath(const std::string &relativePath);
	std::string getCurrentProcessName();
}