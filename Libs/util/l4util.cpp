#include "stdafx.h"

#include "l4util.h"
#include <sstream> 
#include <minwindef.h>

namespace l4util {
	std::string getCurrentExePath()
	{
		char buf[MAX_PATH] = "", *tmp;
		if (GetModuleFileNameA(NULL, buf, MAX_PATH))
		{
			tmp = strrchr(buf, '\\');
			if (tmp) {
				*tmp = '\0';
			}
		}
		return std::string(buf);
	}

	std::string getFileFullPath(const std::string &dllFileName)
	{
		std::ostringstream result;
		result << getCurrentExePath().c_str() << "\\" << dllFileName.c_str();
		return result.str();
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
}