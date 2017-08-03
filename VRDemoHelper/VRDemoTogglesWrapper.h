#pragma once

#include "VRDemoArbiter.h"
#include "VRDemoEventDispatcher.h"
#include "VRDemoNotificationManager.h"
#include "VRDemoSteamVRConfigurator.h"
#include "VRDemoConfigurator.h"
#include "VRDemoHelper.h"

extern bool backgroundMode;

// TODO: refactor it via a CPP file
class VRDemoTogglesWrapper {
public:
    inline bool getPause() { return m_toggles.m_pause; };
    inline bool getMaximmizeGames() { return m_toggles.m_maximizeGames; };
    inline bool getImproveSteamVR() { return m_toggles.m_improveSteamVR; };
    inline bool getShowFPS() { return m_toggles.m_showFPS; };
    inline bool getValue(int toggleIndex) {
        assert(VRDemoArbiter::TI_MIN <= toggleIndex && VRDemoArbiter::TI_MAX >= toggleIndex);
        if (VRDemoArbiter::TI_MIN <= toggleIndex && VRDemoArbiter::TI_MAX >= toggleIndex) {
            return m_toggles.m_values[toggleIndex];
        }
        else {
            return false;
        }
    }
    inline void setValue(int toggleIndex, bool value) {
        if (VRDemoArbiter::TI_MIN <= toggleIndex && VRDemoArbiter::TI_MAX >= toggleIndex) {
            bool oldValue = m_toggles.m_values[toggleIndex];

            m_toggles.m_values[toggleIndex] = value;
            if (oldValue != value) {
                VRDemoEventDispatcher::getInstance().dispatchEvent(
                    VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED,
                    toggleIndex,
                    m_toggles.m_values[toggleIndex]
                );
            }
        }
    }
    inline void setImproveSteamVR(bool value) {
        setValue(VRDemoArbiter::TI_IMPROVE_STEAM_VR, value);
    }
    inline void setShowFPS(bool value) {
        setValue(VRDemoArbiter::TI_SHOW_FPS, value);
    }
    inline void setMaximizeGames(bool value) {
        setValue(VRDemoArbiter::TI_MAXIMIZE_GAMES, value);
    }
    inline bool toggleValue(int toggleIndex) {
        bool newValue = !getValue(toggleIndex);
        setValue(toggleIndex, newValue);
        return newValue;
    }
    inline void togglePause() {
        if (!backgroundMode) {
            // quick hack here, no event listener used
            VRDemoNotificationManager::getInstance().modifyNotificationIcon(
                toggleValue(VRDemoArbiter::TI_PAUSE)
            );
        }
    }
    inline void toggleMaximizeGamesAndSave() {
         saveValue("MaximizeGames", toggleValue(VRDemoArbiter::TI_MAXIMIZE_GAMES));
    };
    inline void toggleImproveSteamVRAndSave() {
        bool newValue =  toggleValue(VRDemoArbiter::TI_IMPROVE_STEAM_VR);

        if (newValue) {
#if WITH_STEAM_VR_CONFIGURATOR
            VRDemoSteamVRConfigurator::getInstance().applySettings();
#endif // WITH_STEAM_VR_CONFIGURATOR
        }
        else {
            // We'll not restore settings file in running,
            // only restore it when it quits normally.
            //VRDemoSteamVRConfigurator::getInstance().restoreSettings();
        }
        saveValue("ImproveSteamVR", newValue);
    }
    inline void toggleShowFPSAndSave() {
        saveValue("ShowFPS", toggleValue(VRDemoArbiter::TI_SHOW_FPS));
    }
    VRDemoTogglesWrapper() :
        m_toggles({ false, true, true, true }) {
    }
    const VRDemoArbiter::Toggles& getToggles() {
        return m_toggles;
    }
    inline void loadConfig() {
        bool toggleValue;
        VRDemoConfigurator &configurator = VRDemoConfigurator::getInstance();

        if (configurator.getValue(HELPER_SECTION_NAME, "ShowFPS", &toggleValue)) {
            setShowFPS(toggleValue);
        }
        if (configurator.getValue(HELPER_SECTION_NAME, "ImproveSteamVR", &toggleValue)) {
            setImproveSteamVR(toggleValue);
        }
        if (configurator.getValue(HELPER_SECTION_NAME, "MaximizeGames", &toggleValue)) {
            setMaximizeGames(toggleValue);
        }
    }
private:
    inline void saveValue(const std::string& key, bool value) {
        if (!backgroundMode) {
            VRDemoConfigurator::getInstance().saveValue(HELPER_SECTION_NAME, key, value);
        }
    }
    VRDemoArbiter::Toggles m_toggles;
};

