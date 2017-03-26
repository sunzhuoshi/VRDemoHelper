#include "stdafx.h"
#include "SteamVRDemoRuleManager.h"

#include <sstream>

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
};

SteamVRDemoRuleManager::RuleItemList SteamVRDemoRuleManager::s_ruleItemList;

void SteamVRDemoRuleManager::handleMessage(int message, HWND wnd)
{
	char className[MAX_PATH];
	RuleAction action = RA_UNKNOWN;

	RealGetWindowClassA(wnd, className, MAX_PATH);
	for (RuleItemList::const_iterator it = SteamVRDemoRuleManager::s_ruleItemList.begin(); it != SteamVRDemoRuleManager::s_ruleItemList.end(); ++it) {
		if (it->m_className == className && it->m_message == message) {
			action = it->m_action;
		}
	}
	if (RA_UNKNOWN != action) {
		ShowWindow(wnd, action);
	}
}

int SteamVRDemoRuleManager::parseValue(const std::string &token, const TokenMap &tokenMap)
{
	int result = -1;
	std::string fixedToken;

	// trim it
	std::stringstream trimmer;
	trimmer << token;
	trimmer.clear();
	trimmer >> fixedToken;
	// to upper
	for (auto & c : fixedToken) { c = toupper(c); };

	for (TokenMap::const_iterator it = tokenMap.begin(); it != tokenMap.end(); ++it) {
		if (it->first == fixedToken) {
			result = it->second;
			break;
		}
	}
	return result;
}


bool SteamVRDemoRuleManager::parseSection(const std::string &sectionName, const std::string &filePath, RuleItem &ruleItem) {
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
				RuleItem ruleItem;
				if (parseSection(sectionName.str() , configFilePath, ruleItem)) {
					SteamVRDemoRuleManager::s_ruleItemList.push_back(ruleItem);
				}
				else {
					// TODO: log it as a warning
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

