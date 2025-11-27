//
// Created by dkueh on 27.11.2025.
//

#ifndef PROJECT_SIPX_EVENTHANDLER_H
#define PROJECT_SIPX_EVENTHANDLER_H

#include "Event.h"

class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void on_event(const Event& evt) = 0;
};

#endif //PROJECT_SIPX_EVENTHANDLER_H