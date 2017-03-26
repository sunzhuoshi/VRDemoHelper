#pragma once
#include <string>
#include <map>
#include <list>

class SteamVRDemoRuleManager
{
public:
	typedef std::map<std::string, int> TokenMap;
	enum RuleMessage {
		RM_UNKNOWN = -1,
		RM_ACTIVATE = HCBT_CREATEWND,
		RM_CREATE = HCBT_ACTIVATE
	};
	enum RuleAction {
		RA_UNKNOWN = -1,
		RA_MAX = SW_MAXIMIZE,
		RA_MIN = SW_MINIMIZE,
		RA_HIDE = SW_HIDE
	};
	struct RuleItem {
		std::string m_ruleName;
		std::string m_className;
		RuleMessage m_message;
		RuleAction  m_action;
		RuleItem() : m_message(RM_UNKNOWN),
			m_action(RA_UNKNOWN)
		{};
		bool isValid() {
			return m_ruleName.length() > 0 &&
				m_className.length() > 0 &&
				m_message != RM_UNKNOWN &&
				m_action != RA_UNKNOWN;
		};
	};
	typedef std::list<RuleItem> RuleItemList;
	static const TokenMap s_ruleMessageTokenMap, s_ruleActionTokenMap;
	static RuleItemList s_ruleItemList;
	static void handleMessage(int message, HWND wnd);
	static int parseValue(const std::string &token, const TokenMap &tokenMap);
	static bool parseSection(const std::string &sectionName, const std::string &filePath, RuleItem &ruleItem);
	static bool init(const char *configFilePath);
};

