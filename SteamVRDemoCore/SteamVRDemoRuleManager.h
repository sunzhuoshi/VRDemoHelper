#pragma once
#include <string>
#include <map>
#include <list>

class SteamVRDemoRuleManager
{
public:
	enum RuleMessage {
		RM_UNKNOWN = -1,
		RM_ACTIVATE = HCBT_CREATEWND,
		RM_CREATE = HCBT_ACTIVATE
	};
	enum RuleAction {
		RA_UNKNOWN = -1,
		RA_MAX = SW_MAXIMIZE,
		RA_MIN = SW_MINIMIZE,
		RA_HIDE = SW_HIDE,
		RA_FULL = SW_MAX + 1,
		RA_CLOSE = WM_CLOSE,
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
	typedef std::map<std::string, int> TokenMap;
	static const TokenMap s_ruleMessageTokenMap, s_ruleActionTokenMap;
	typedef std::list<std::string> NameList;
	static NameList s_ignoredProcessNameList;
	typedef std::list<RuleItem> RuleItemList;
	static RuleItemList s_ruleItemList;
	static bool ifIgnore(const std::string &processName);
	static void performFullScreenAction(HWND wnd);
	static void handleMessage(int message, HWND wnd);
	static int parseValue(const std::string &token, const TokenMap &tokenMap);
	static bool parseIgnoreListSection(const std::string &filePath);
	static bool parseRuleSection(const std::string &sectionName, const std::string &filePath, RuleItem &ruleItem);
	static bool init(const char *configFilePath);
};

