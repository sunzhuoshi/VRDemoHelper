#pragma once

#include "VRDemoArbiter.h"
#include "VRDemoEventDispatcher.h"

class VRDemoTogglesWrapper {
public:
    inline BOOL getPause() { return m_toggles.m_pause; };
    inline BOOL getMaximmizeGames() { return m_toggles.m_maximizeGames; };
    inline BOOL getHideSteamVrNotification() { return m_toggles.m_hideSteamVrNotifcation; };
    inline BOOL togglePause() {
        m_toggles.m_pause = !m_toggles.m_pause;
        VRDemoEventDispatcher::getInstance().dispatchEvent(
            VRDemoEventDispatcher::EV_PAUSE_CHANGED,
            m_toggles.m_pause
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
    VRDemoTogglesWrapper() :
        m_toggles({ FALSE, TRUE, TRUE }) {
    }
    const VRDemoArbiter::Toggles& getToggles() {
        return m_toggles;
    }
private:
    VRDemoArbiter::Toggles m_toggles;
};

