#include "stdafx.h"
#include "SteamVRDemoRuleManager.h"

#include <string.h>
#include <sstream>
#include <log4cplus\log4cplus.h>
#include "SteamVRDemoUtil.h"

const char *IGNORE_LIST_SECTION = "IgnoreList";

enum RuleMessage {
	RM_UNKNOWN = -1,
	RM_ACTIVATE = HCBT_ACTIVATE,
	RM_CREATE = HCBT_CREATEWND
};
enum RuleAction {
	RA_UNKNOWN = -1,
	RA_MAX = SW_MAXIMIZE,
	RA_MIN = SW_MINIMIZE,
	RA_HIDE = SW_HIDE
};

const SteamVRDemoRuleManager::TokenMap SteamVRDemoRuleManager::s_ruleMessageTokenMap = {
	{"CREATE", HCBT_CREATEWND},
	{"ACTIVATE", HCBT_ACTIVATE }
};

const SteamVRDemoRuleManager::TokenMap SteamVRDemoRuleManager::s_ruleActionTokenMap = {
	{"MAX", SW_MAXIMIZE },
	{"MIN", SW_MINIMIZE },
	{"HIDE", SW_HIDE },
	{"CLOSE", WM_CLOSE},
	{"FULL", SteamVRDemoRuleManager::RA_FULL}
};

SteamVRDemoRuleManager::RuleItemList SteamVRDemoRuleManager::s_ruleItemList;
SteamVRDemoRuleManager::NameList SteamVRDemoRuleManager::s_ignoredProcessNameList;

// TODO: move it into a common file to share
std::string Trim(const std::string &str)
{
	std::string result;
	std::stringstream trimmer;

	trimmer << str;
	trimmer.clear();
	trimmer >> result;
	return result;
}

// TODO: move it into a common file to share
std::vector<std::string>& split(const std::string &text, char sep, std::vector<std::string> &tokens) {
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

// TODO: move it into a common file to share
typedef std::pair<std::string, std::string> KeyValuePair;
bool ParseKeyValue(const std::string &line, KeyValuePair &keyValue)
{
	bool result = false;
	std::string trimmedLine = Trim(line);
	std::vector<std::string> splited;
	split(line, '=', splited);
	if (2 == splited.size()) {
		keyValue.first = splited[0];
		keyValue.second = splited[1];
		result = true;
	}
	return result;
}

bool SteamVRDemoRuleManager::ifIgnore(const std::string &processName)
{
	bool result = false;

	for (NameList::const_iterator it = SteamVRDemoRuleManager::s_ignoredProcessNameList.begin(); it != SteamVRDemoRuleManager::s_ignoredProcessNameList.end(); ++it) {
		if (0 == _stricmp(processName.c_str(), it->c_str())) {
			result = true;
			break;
		}
	}
	return result;
}

void SteamVRDemoRuleManager::performFullScreenAction(HWND wnd)
{
	HWND desktopWindow = GetDesktopWindow();
	RECT desktopRect, windowRect;
	log4cplus::Logger logger = log4cplus::Logger::getInstance("SERVER");

	GetWindowRect(desktopWindow, &desktopRect);
	GetWindowRect(wnd, &windowRect);

	LOG4CPLUS_DEBUG(logger, "Desktop width: " << desktopRect.right << ", height: " << desktopRect.bottom);
	LOG4CPLUS_DEBUG(logger, "Window width: " << windowRect.right - windowRect.left << ", height: " << desktopRect.bottom - desktopRect.top);

	if (windowRect.left > desktopRect.left ||
		windowRect.right < desktopRect.right ||
		windowRect.top < desktopRect.top ||
		windowRect.bottom < desktopRect.bottom) {
		LOG4CPLUS_DEBUG(logger, "Window is not full screen size, toggole it");
		// TODO: check why it desn't work with Unreal 4 games(works with Unity games)
		PostMessageA(wnd, WM_SYSKEYDOWN, VK_RETURN, 1 << 29 | 0x001C0001);
		PostMessageA(wnd, WM_SYSCHAR, 13, 1 << 29 | 0x001C0001);
	}
}

void SteamVRDemoRuleManager::handleMessage(int message, HWND wnd)
{
	char className[MAX_PATH];
	RuleAction action = RA_UNKNOWN;

	log4cplus::Logger logger = log4cplus::Logger::getInstance("SERVER");
	DWORD processId = GetCurrentProcessId();
	std::string processName = steam_vr_demo_helper::getCurrentProcessName();
	std::string messageStr;

	if (SteamVRDemoRuleManager::ifIgnore(processName)) {
		return;
	}

	RealGetWindowClassA(wnd, className, MAX_PATH);
	for (RuleItemList::const_iterator it = SteamVRDemoRuleManager::s_ruleItemList.begin(); it != SteamVRDemoRuleManager::s_ruleItemList.end(); ++it) {
		if (it->m_className == className && it->m_message == message) {
			action = it->m_action;
		}
	}
	// TODO: refactor to make it extendable
	switch (message)
	{
	case HCBT_ACTIVATE:
		messageStr = "ACTIVATE";
		break;
	case HCBT_CREATEWND:
		messageStr = "CREATE";
		break;
	default:
		break;
	}
	LOG4CPLUS_INFO(logger, "[" << messageStr << "] ID=" << processId << ", name=" << processName << ", Class=" << className << std::endl);

	if (RA_UNKNOWN != action) {
		switch (action) {
		case RA_FULL:
			performFullScreenAction(wnd);
			break;
		case RA_CLOSE:
			PostMessage(wnd, WM_CLOSE, 0, 0);
			break;
		default:
			// show window commands
			ShowWindow(wnd, action);
			break;
		}
	}
}

int SteamVRDemoRuleManager::parseValue(const std::string &token, const TokenMap &tokenMap)
{
	int result = -1;
	std::string trimmedToken = Trim(token);

	for (TokenMap::const_iterator it = tokenMap.begin(); it != tokenMap.end(); ++it) {
		if (0 == _stricmp(it->first.c_str(), trimmedToken.c_str())) {
			result = it->second;
			break;
		}
	}
	return result;
}

bool SteamVRDemoRuleManager::parseIgnoreListSection(const std::string &filePath) {
	bool result = false;
	char buf[1024] = "";
	DWORD p = 0, ret = GetPrivateProfileSectionA(IGNORE_LIST_SECTION, buf, sizeof(buf), filePath.c_str());
	std::ostringstream line;
	while (p < ret) {
		line << buf[p];
		if ('\0' == buf[p]) {
			KeyValuePair keyValue;
			if (ParseKeyValue(line.str(), keyValue)) {
				s_ignoredProcessNameList.push_back(keyValue.second);
			}
			line.str("");
			line.clear();
		}
		p++;
	}
	return result;
}

bool SteamVRDemoRuleManager::parseRuleSection(const std::string &sectionName, const std::string &filePath, RuleItem &ruleItem) {
	bool result = true;
	char value[MAX_PATH];

	result &= (0 < GetPrivateProfileStringA(sectionName.c_str(), "ClassName", "", value, sizeof(value), filePath.c_str()));
	ruleItem.m_className = value;

	result &= (0 < GetPrivateProfileStringA(sectionName.c_str(), "Message", "", value, sizeof(value), filePath.c_str()));
	ruleItem.m_message = (RuleMessage)parseValue(value, SteamVRDemoRuleManager::s_ruleMessageTokenMap);
	
	result &= (0 < GetPrivateProfileStringA(sectionName.c_str(), "Action", "", value, sizeof(value), filePath.c_str()));
	ruleItem.m_action = (RuleAction)parseValue(value, SteamVRDemoRuleManager::s_ruleActionTokenMap);

	return result;
}

bool SteamVRDemoRuleManager::init(const char *configFilePath)
{
	bool result = true;
	char buf[1024];
	DWORD p = 0, ret = GetPrivateProfileSectionNamesA(buf, sizeof(buf), configFilePath);
	if (0 < ret) {
		std::ostringstream sectionName;
		while (p < ret) {
			sectionName << buf[p];
			if ('\0' == buf[p]) {
				if (0 == _stricmp(IGNORE_LIST_SECTION, sectionName.str().c_str())) {
					parseIgnoreListSection(configFilePath);
				}
				else {
					RuleItem ruleItem;
					if (parseRuleSection(sectionName.str(), configFilePath, ruleItem)) {
						SteamVRDemoRuleManager::s_ruleItemList.push_back(ruleItem);
					}
					else {
						// TODO: log it as a warning
					}
				}
				sectionName.str("");
				sectionName.clear();
			}
			p++;
		}
	}
	else {
		result = false;
	}
	return result;
}

