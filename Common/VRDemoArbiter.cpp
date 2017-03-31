#include "stdafx.h"
#include "VRDemoArbiter.h"

#include <string.h>
#include <sstream>
#include <log4cplus\log4cplus.h>
#include "util\l4util.h"

const std::string VRDemoArbiter::IGNORE_LIST_SECTION = "IgnoreList";
const std::string VRDemoArbiter::RULE_CONFIG_FILE = "rule_config.ini";
const std::string VRDemoArbiter::PREFIX_HIDE_STEAM_VR_NOTIFICATION = "SteamVR";
const std::string VRDemoArbiter::PREFIX_MAXIMIZE_GAMES = "Game";

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleTypeTokenMap = {
    {VRDemoArbiter::RT_MESSAGE, "MESSAGE"},
    {VRDemoArbiter::RT_POLL, "POLL"},
    {VRDemoArbiter::RT_UNKNOWN, "UNKNOWN"}
};

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleMessageTokenMap = {
    {HCBT_ACTIVATE, "ACTIVATE"},
    {HCBT_CREATEWND, "CREATE"},
    {VRDemoArbiter::RM_UNKNOWN, "UNKNOWN"}
};

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleActionTokenMap = {
	{SW_MAXIMIZE, "MAX"},
	{SW_MINIMIZE, "MIN"},
	{SW_HIDE, "HIDE"},
	{WM_CLOSE, "CLOSE"},
	{VRDemoArbiter::RA_FULL, "FULL"},
    {VRDemoArbiter::RA_UNKNOWN, "UNKNOWN"}
};

bool VRDemoArbiter::arbitrate(RuleType type, int message, HWND wnd)
{
    bool result = false;
    char className[MAX_PATH];
    RuleAction action = RA_UNKNOWN;
    DWORD processId = 0;

    GetWindowThreadProcessId(wnd, &processId);

    std::string processName = l4util::getProcessNameWithWindow(wnd);

    // if we can't get process name, then we can do less...
    if (!ifIgnore(processName) && processName.size()) {
        RealGetWindowClassA(wnd, className, MAX_PATH);

        LOG4CPLUS_INFO(m_logger, "[ " << VRDemoArbiter::s_ruleTypeTokenMap[type] << " | " << VRDemoArbiter::s_ruleMessageTokenMap[message] << " ] ID=" << processId << ", name=" << processName << ", Class=" << className << std::endl);

        RuleItemMap::const_iterator it = m_ruleItemMap.begin();
        while (it != m_ruleItemMap.end()) {
            bool okToGo = false;
            if (l4util::keyStartWith(it->first, VRDemoArbiter::PREFIX_MAXIMIZE_GAMES)) {
                okToGo = m_maximizeGames;
            }
            else if (l4util::keyStartWith(it->first, VRDemoArbiter::PREFIX_HIDE_STEAM_VR_NOTIFICATION)) {
                okToGo = m_hideSteamVrNotifcation;
            }
            else {
                okToGo = true;
            }
            if (okToGo && type == it->second.m_type &&
                    0 == it->second.m_className.compare(className) &&
                    it->second.getMessage() == message) {
                    action = it->second.m_action;
                    break;
            }
            ++it;
        }
        if (it != m_ruleItemMap.end()) {
            performAction(wnd, it->second);
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
    LOG4CPLUS_WARN(m_logger, "Performing action, rule item: " << ruleItem.toString());
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
        LOG4CPLUS_WARN(m_logger, "Unknown action, rule item: " << ruleItem.toString());
        break;
    }
}


void VRDemoArbiter::performFullScreenAction(HWND wnd, const RuleItem &ruleItem)
{
	HWND desktopWindow = GetDesktopWindow();
	RECT desktopRect, windowRect;

	GetWindowRect(desktopWindow, &desktopRect);
	GetWindowRect(wnd, &windowRect);

	LOG4CPLUS_DEBUG(m_logger, "Desktop width: " << desktopRect.right << ", height: " << desktopRect.bottom);
	LOG4CPLUS_DEBUG(m_logger, "Window width: " << windowRect.right - windowRect.left << ", height: " << desktopRect.bottom - desktopRect.top);

	if (windowRect.left > desktopRect.left ||
		windowRect.right < desktopRect.right ||
		windowRect.top < desktopRect.top ||
		windowRect.bottom < desktopRect.bottom) {
		LOG4CPLUS_DEBUG(m_logger, "Window is not full screen size, toggole it");
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

    // if not empty(parse), ignore the later ones
    if (m_ignoredProcessNameList.empty()) {
        char buf[1024] = "";
        DWORD p = 0, ret = GetPrivateProfileSectionA(VRDemoArbiter::IGNORE_LIST_SECTION.c_str(), buf, sizeof(buf), filePath.c_str());
        std::ostringstream line;

        while (p < ret) {
            line << buf[p];
            if ('\0' == buf[p]) {
                l4util::StringPair keyValue;
                if (l4util::parseProperty(line.str(), keyValue)) {
                    if (std::string::npos == keyValue.first.find('#')) {
                        m_ignoredProcessNameList.push_back(keyValue.second);
                    }
                }
                line.str("");
                line.clear();
                result = true;
            }
            p++;
        }
    }
    else {
        LOG4CPLUS_WARN(m_logger, "Ignore process name list already defined, check your config file: \n" << filePath);
    }
	return result;
}

bool VRDemoArbiter::parseRuleSection(const std::string &sectionName, const std::string &filePath, RuleItem &ruleItem) {
	bool result = false;
	char buf[1024];
    DWORD p = 0, ret = GetPrivateProfileSectionA(sectionName.c_str(), buf, sizeof(buf), filePath.c_str());

    if (ret <= sizeof(buf) - 2) {
        std::ostringstream line;
        RuleItem tmpRuleItem;
        tmpRuleItem.m_ruleName = sectionName;
        while (p < ret) {
            if ('\0' != buf[p]) {
                line << buf[p];
            }
            else {
                l4util::StringPair keyValue;
                if (l4util::parseProperty(line.str(), keyValue)) {
                    if (l4util::matchKey(keyValue.first, "ClassName")) {
                        tmpRuleItem.m_className = keyValue.second;
                    }
                    else if (l4util::matchKey(keyValue.first, "Type")) {
                        tmpRuleItem.m_type = (RuleType)parseValue(keyValue.second, VRDemoArbiter::s_ruleTypeTokenMap);
                    }
                    else if (l4util::matchKey(keyValue.first, "Message")) {
                        tmpRuleItem.setMessage((RuleMessage)parseValue(keyValue.second, VRDemoArbiter::s_ruleMessageTokenMap));
                    }
                    else if (l4util::matchKey(keyValue.first, "Action")) {
                        tmpRuleItem.m_action = (RuleAction)parseValue(keyValue.second, VRDemoArbiter::s_ruleActionTokenMap);
                    }
                }
                line.str("");
                line.clear();
            }
            p++;
        }
        if (tmpRuleItem.isValid()) {
            ruleItem = tmpRuleItem;
            result = true;
        }
    }
    else {
        LOG4CPLUS_WARN(m_logger, "Section too large: " << sectionName);
    }
	return result;
}

bool VRDemoArbiter::init(const std::string &configFilePath, const std::string &loggerName)
{
	bool result = true;
	char buf[1024];
	DWORD p = 0, ret = GetPrivateProfileSectionNamesA(buf, sizeof(buf), configFilePath.c_str());
    log4cplus::Logger logger = log4cplus::Logger::getInstance(loggerName);

    LOG4CPLUS_DEBUG(logger, "Initializing VR Demo Arbitar, file path: " << configFilePath);
	if (0 < ret) {
		std::ostringstream sectionName;
		while (p < ret) {
            if ('\0' != buf[p]) {
                sectionName << buf[p];
            } 
            else {
                std::string sectionKey = l4util::toUpper(sectionName.str());
				if (0 == _stricmp(VRDemoArbiter::IGNORE_LIST_SECTION.c_str(), sectionName.str().c_str())) {
					parseIgnoreListSection(configFilePath);
				}
				else {
					RuleItem ruleItem;
                    if (!m_ruleItemMap.count(sectionKey)) {
                        if (parseRuleSection(sectionName.str(), configFilePath, ruleItem)) {
                            m_ruleItemMap[sectionKey] = ruleItem;
                        }
                        else {
                            LOG4CPLUS_WARN(logger, "Invalid section: " << sectionName.str());
                        }
                    }
                    else {
                        LOG4CPLUS_WARN(logger, "Duplicated section name: " << sectionName.str() << ", only the first one will be used");
                    }
				}
				sectionName.str("");
				sectionName.clear();
            }
			p++;
		}
        m_configFilePath = configFilePath;
        m_logger = logger;
        LOG4CPLUS_DEBUG(logger, "VR Demo Arbitar inited");
	}
	else {
		result = false;
	}
	return result;
}

