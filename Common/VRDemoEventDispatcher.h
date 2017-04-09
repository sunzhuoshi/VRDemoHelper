#pragma once
#include <list>
#include <map>
#include <log4cplus\helpers\pointer.h>

/*
    Event listener interface
    Note: We need to refer and hold the life of the listeners
*/
class IVRDemoEventListener : virtual public log4cplus::helpers::SharedObject
{
public:
    virtual void handleEvent(int event, unsigned long long param) = 0;
};

/*
    Event dispatcher
    Note: not thread safe, needed?
*/
class VRDemoEventDispatcher
{
public:
    enum Event {
        EV_PAUSE_CHANGED,
        EV_MAXIMIZE_GAMES_CHANGED,
        EV_HIDE_STEAM_VR_NOTIFICATION_CHANGED,
        EV_SHOW_FPS_CHANGED,
        EV_ALL     // for lazy users like me
    };
    typedef log4cplus::helpers::SharedObjectPtr<IVRDemoEventListener> VRDemoEventListenerPtr;
    typedef std::list<VRDemoEventListenerPtr> VRDemoEventListenerList;
    typedef std::map<int, VRDemoEventListenerList> VRDemoEventListenersMap;

    VRDemoEventDispatcher();
    ~VRDemoEventDispatcher();
    static VRDemoEventDispatcher& getInstance() {
        static VRDemoEventDispatcher instance;
        return instance;
    }
    void addEventListener(int event, VRDemoEventListenerPtr listener);
    void dispatchEvent(int event, unsigned long long param);
private:
    VRDemoEventListenersMap m_eventListenersMap;
    VRDemoEventListenerList m_globalEventListeners;
};

