#pragma once
#include <sstream>
#include <string>
#include <map>
#include <list>

class VRDemoArbiter
{
public:
	static const int UNKNOWN_TOKEN = -1;
	enum RuleMessage {
		RM_UNKNOWN = UNKNOWN_TOKEN,
		RM_ACTIVATE = HCBT_CREATEWND,
		RM_CREATE = HCBT_ACTIVATE
	};
	enum RuleAction {
		RA_UNKNOWN = UNKNOWN_TOKEN,
		RA_MAX = SW_MAXIMIZE,
		RA_MIN = SW_MINIMIZE,
		RA_HIDE = SW_HIDE,
		RA_FULL = SW_MAX + 1,
		RA_CLOSE = WM_CLOSE,
	};
    enum RuleType {
        RT_UNKNOWN = UNKNOWN_TOKEN,
        RT_MESSAGE,
        RT_POLL
    };
	struct RuleItem {
		std::string m_ruleName;
		std::string m_className;
        RuleType    m_type;
		RuleMessage m_message;
		RuleAction  m_action;
		RuleItem() : m_message(RM_UNKNOWN),
			m_action(RA_UNKNOWN),
            m_type(RT_UNKNOWN)
		{};
		bool isValid() {
			return m_ruleName.length() > 0 &&
				m_className.length() > 0 &&
				m_message != RM_UNKNOWN &&
				m_action != RA_UNKNOWN;
        };
        std::string toString() const {
            std::ostringstream buf;
            buf << "{ name: " << m_ruleName <<
                ", className: " << m_className <<
                ", type: " << s_ruleTypeTokenMap[m_type] <<
                ", message: " << s_ruleMessageTokenMap[m_message] <<
                ", action: " << s_ruleActionTokenMap[m_action];
            return buf.str();
        };
	};
private:
    typedef std::map<int, std::string> TokenMap;
    typedef std::list<std::string> NameList;
    typedef std::list<RuleItem> RuleItemList;
public:
    VRDemoArbiter::VRDemoArbiter() {};
    VRDemoArbiter::~VRDemoArbiter() {};
    static VRDemoArbiter& VRDemoArbiter::getInstance() {
        static VRDemoArbiter instance;
        return instance;
    }
    bool init(const std::string &configFilePath);
    bool arbitrate(RuleType type, int message, HWND wnd);
private:
	bool ifIgnore(const std::string &processName);

    void performAction(HWND wnd, const RuleItem &ruleItems);
	void performFullScreenAction(HWND wnd, const RuleItem &ruleItem);
    void performShowWindowAction(HWND wnd, const RuleItem &ruleItem);

    int parseValue(const std::string &token, const TokenMap &tokenMap);
	bool parseIgnoreListSection(const std::string &filePath);
	bool parseRuleSection(const std::string &sectionName, const std::string &filePath, RuleItem &ruleItem);

    static TokenMap s_ruleTypeTokenMap;
    static TokenMap s_ruleMessageTokenMap;
    static TokenMap s_ruleActionTokenMap;
    static const std::string IGNORE_LIST_SECTION;

    NameList m_ignoredProcessNameList;
    RuleItemList m_ruleItemList;
    std::string m_configFilePath;
};

