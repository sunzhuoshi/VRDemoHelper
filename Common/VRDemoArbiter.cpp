#include "stdafx.h"
#include "VRDemoArbiter.h"

#include <string.h>
#include <sstream>
#include <log4cplus\log4cplus.h>
#include "util\l4util.h"

const std::string VRDemoArbiter::IGNORE_LIST_SECTION = "IgnoreList";

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleTypeTokenMap = {
    {VRDemoArbiter::RT_MESSAGE, "MESSAGE"},
    {VRDemoArbiter::RT_POLL, "POLL"},
    {VRDemoArbiter::RT_UNKNOWN, "UNKNOWN"}
};

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleMessageTokenMap = {
	{HCBT_ACTIVATE, "ACTIVATE"},
	{HCBT_CREATEWND, "CREATE"}
};

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleActionTokenMap = {
	{SW_MAXIMIZE, "MAX"},
	{SW_MINIMIZE, "MIN"},
	{SW_HIDE, "HIDE"},
	{WM_CLOSE, "CLOSE"},
	{VRDemoArbiter::RA_FULL, "FULL"}
};

bool VRDemoArbiter::arbitrate(RuleType type, int message, HWND wnd)
{
    bool result = false;
    char className[MAX_PATH];
    RuleAction action = RA_UNKNOWN;

    log4cplus::Logger logger = log4cplus::Logger::getInstance("SERVER");
    DWORD processId = GetCurrentProcessId();
    std::string processName = l4util::getCurrentProcessName();

    if (!ifIgnore(processName)) {
        RealGetWindowClassA(wnd, className, MAX_PATH);

        LOG4CPLUS_INFO(logger, "[" << VRDemoArbiter::s_ruleMessageTokenMap[message] << "] ID=" << processId << ", name=" << processName << ", Class=" << className << std::endl);

        RuleItemList::const_iterator it = m_ruleItemList.begin();

        while (it != m_ruleItemList.end()) {
            if (type == it->m_type && it->m_className == className && it->m_message == message) {
                action = it->m_action;
                break;
            }
        }
        if (it != m_ruleItemList.end()) {
            performAction(wnd, *it);
            result = true;
        }
    }
    return result;
}


bool VRDemoArbiter::ifIgnore(const std::string &processName)
{
	bool result = false;

	for (NameList::const_iterator it = m_ignoredProcessNameList.begin(); it != m_ignoredProcessNameList.end(); ++it) {
		if (0 == _stricmp(processName.c_str(), it->c_str())) {
			result = true;
			break;
		}
	}
	return result;
}

void VRDemoArbiter::performAction(HWND wnd, const RuleItem &ruleItem)
{
    switch (ruleItem.m_action) {
    case RA_FULL:
        performFullScreenAction(wnd, ruleItem);
        break;
    case RA_CLOSE:
        PostMessage(wnd, WM_CLOSE, 0, 0);
        break;
    case RA_HIDE:
    case RA_MAX:
    case RA_MIN:
        // show window commands
        ShowWindow(wnd, ruleItem.m_action);
        break;
    default:
        LOG4CPLUS_WARN(log4cplus::Logger::getInstance("SERVER"), "Unknown action, rule item: " << ruleItem.toString());
        break;
    }
}


void VRDemoArbiter::performFullScreenAction(HWND wnd, const RuleItem &ruleItem)
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
		PostMessageA(wnd, WM_KEYDOWN, VK_MENU, 0);				// Post WM_KEYDOWN for Unreal games
		// TODO: check if char code or repeat is necessary
		PostMessageA(wnd, WM_SYSKEYDOWN, VK_RETURN, 1 << 29 | 0x001C0001);  // ALT down | char code = 1C | repeat = 1
		PostMessageA(wnd, WM_KEYUP, VK_MENU, 0);				// Post WM_KEYUP for Unreal games  
	}
}

int VRDemoArbiter::parseValue(const std::string &token, const TokenMap &tokenMap)
{
	int result = UNKNOWN_TOKEN;
	std::string trimmedToken = (token);

	for (TokenMap::const_iterator it = tokenMap.begin(); it != tokenMap.end(); ++it) {
		if (0 == _stricmp(it->second.c_str(), trimmedToken.c_str())) {
			result = it->first;
			break;
		}
	}
	return result;
}

bool VRDemoArbiter::parseIgnoreListSection(const std::string &filePath) {
	bool result = false;
	char buf[1024] = "";
	DWORD p = 0, ret = GetPrivateProfileSectionA(VRDemoArbiter::IGNORE_LIST_SECTION.c_str(), buf, sizeof(buf), filePath.c_str());
	std::ostringstream line;
	while (p < ret) {
		line << buf[p];
		if ('\0' == buf[p]) {
			l4util::StringPair keyValue;
			if (l4util::parseProperty(line.str(), keyValue)) {
				m_ignoredProcessNameList.push_back(keyValue.second);
			}
			line.str("");
			line.clear();
		}
		p++;
	}
	return result;
}

bool VRDemoArbiter::parseRuleSection(const std::string &sectionName, const std::string &filePath, RuleItem &ruleItem) {
	bool result = true;
	char value[MAX_PATH];

	// TODO: refactor it with GetPrivateProfileSection
	result &= (0 < GetPrivateProfileStringA(sectionName.c_str(), "ClassName", "", value, sizeof(value), filePath.c_str()));
	ruleItem.m_className = value;

	result &= (0 < GetPrivateProfileStringA(sectionName.c_str(), "Message", "", value, sizeof(value), filePath.c_str()));
	ruleItem.m_message = (RuleMessage)parseValue(value, VRDemoArbiter::s_ruleMessageTokenMap);
	
	result &= (0 < GetPrivateProfileStringA(sectionName.c_str(), "Action", "", value, sizeof(value), filePath.c_str()));
	ruleItem.m_action = (RuleAction)parseValue(value, VRDemoArbiter::s_ruleActionTokenMap);

	return result;
}

bool VRDemoArbiter::init(const std::string &configFilePath)
{
	bool result = true;
	char buf[1024];
	DWORD p = 0, ret = GetPrivateProfileSectionNamesA(buf, sizeof(buf), configFilePath.c_str());
	if (0 < ret) {
		std::ostringstream sectionName;
		while (p < ret) {
			sectionName << buf[p];
			if ('\0' == buf[p]) {
				if (0 == _stricmp(VRDemoArbiter::IGNORE_LIST_SECTION.c_str(), sectionName.str().c_str())) {
					parseIgnoreListSection(configFilePath);
				}
				else {
					RuleItem ruleItem;
					if (parseRuleSection(sectionName.str(), configFilePath, ruleItem)) {
						m_ruleItemList.push_back(ruleItem);
					}
					else {
                        LOG4CPLUS_WARN(log4cplus::Logger::getInstance("SERVER"), "Invalid section: " << sectionName.str());
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

