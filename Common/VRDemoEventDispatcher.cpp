#include "stdafx.h"
#include "VRDemoEventDispatcher.h"


VRDemoEventDispatcher::VRDemoEventDispatcher()
{
}


VRDemoEventDispatcher::~VRDemoEventDispatcher()
{
}


void VRDemoEventDispatcher::addEventListener(int event, VRDemoEventListenerPtr listener)
{
        VRDemoEventListenersMap::iterator it = m_eventListenersMap.find(event);

        if (it == m_eventListenersMap.end()) {
            m_eventListenersMap[event] = {};
        }
        VRDemoEventListenerList &eventList = m_eventListenersMap[event];
        eventList.push_back(listener);
}

void VRDemoEventDispatcher::dispatchEvent(int event, unsigned long long param1, unsigned long long param2)
{
    VRDemoEventListenersMap::iterator mit = m_eventListenersMap.find(event);

    if (mit != m_eventListenersMap.end()) {
        VRDemoEventListenerList &eventList = mit->second;
        for (VRDemoEventListenerList::iterator lit = eventList.begin(); lit != eventList.end(); ++lit) {
            (*lit)->handleEvent(event, param1, param2);
        }
    }
}
