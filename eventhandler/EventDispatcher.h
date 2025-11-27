//
// Created by dkueh on 27.11.2025.
//

#ifndef PROJECT_SIPX_EVENTDISPATCHER_H
#define PROJECT_SIPX_EVENTDISPATCHER_H

#include "Event.h"
#include "EventHandler.h"

#include <vector>


class EventDispatcher {
public:
    std::vector<EventHandler*> listeners;
    void register_listener(EventHandler* l) {
        listeners.push_back(l);
    }

    void dispatch(const Event& evt) const {
        for (auto* l : listeners) {
            l->on_event(evt);
        }
    }
};


#endif //PROJECT_SIPX_EVENTDISPATCHER_H