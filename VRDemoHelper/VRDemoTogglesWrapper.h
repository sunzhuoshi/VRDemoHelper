#pragma once

#include "VRDemoArbiter.h"
#include "VRDemoEventDispatcher.h"
#include "VRDemoNotificationManager.h"

class VRDemoTogglesWrapper {
public:
    inline bool getPause() { return 0 != m_toggles.m_pause; };
    inline bool getMaximmizeGames() { return 0 != m_toggles.m_maximizeGames; };
    inline bool getHideSteamVrNotification() { return 0 != m_toggles.m_hideSteamVrNotifcation; };
    inline bool getShowFPS() { return 0 != m_toggles.m_showFPS; };
    inline BOOL togglePause() {
        m_toggles.m_pause = !m_toggles.m_pause;
        VRDemoEventDispatcher::getInstance().dispatchEvent(
            VRDemoEventDispatcher::EV_PAUSE_CHANGED,
            m_toggles.m_pause
        );
        VRDemoNotificationManager::getInstance().modifyNotificationIcon(
            0 != m_toggles.m_pause
        );
        return m_toggles.m_pause;
    }
    inline void toggleMaximizeGames() {
        m_toggles.m_maximizeGames = !m_toggles.m_maximizeGames;
        VRDemoEventDispatcher::getInstance().dispatchEvent(
            VRDemoEventDispatcher::EV_MAXIMIZE_GAMES_CHANGED,
            m_toggles.m_maximizeGames
        );
    };
    inline void toggleHideSteamVrNotification() {
        m_toggles.m_hideSteamVrNotifcation = !m_toggles.m_hideSteamVrNotifcation;
        VRDemoEventDispatcher::getInstance().dispatchEvent(
            VRDemoEventDispatcher::EV_HIDE_STEAM_VR_NOTIFICATION_CHANGED,
            m_toggles.m_hideSteamVrNotifcation
        );
    }
    inline void toggleShowFPS() {
        m_toggles.m_showFPS = !m_toggles.m_showFPS;
        VRDemoEventDispatcher::getInstance().dispatchEvent(
            VRDemoEventDispatcher::EV_SHOW_FPS_CHANGED,
            m_toggles.m_showFPS
        );
    }
    VRDemoTogglesWrapper() :
        m_toggles({ FALSE, TRUE, TRUE, TRUE }) {
    }
    const VRDemoArbiter::Toggles& getToggles() {
        return m_toggles;
    }
private:
    VRDemoArbiter::Toggles m_toggles;
};

