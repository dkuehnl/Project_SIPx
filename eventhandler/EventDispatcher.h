/*
*  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. It provides
 *  developers and DevOps engineers fine-grained control over SIP behavior,
 *  including non-RFC-compliant flows and corner-case simulations.
 *
 *  Description of EventDispatcher:
 *  -------------------------------
 *  The EventDispatcher is a lightweight component used for broadcasting events
 *  between SIPx modules. It implements a minimal publish/subscribe mechanism,
 *  allowing any number of EventHandler instances to register themselves and
 *  receive Event objects.
 *
 *  Main responsibilities:
 *    • Maintain a list of registered EventHandler listeners
 *    • Dispatch Event objects to all listeners in registration order
 *
 *  Internal structure:
 *    • Stores raw pointers to EventHandler instances (ownership is not managed)
 *    • Callers are responsible for ensuring the lifetime of handlers
 *
 *  Purpose and usage:
 *    This dispatcher enables loose coupling between modules such as the parser,
 *    logwriter, and future SIP logic components. It allows modules to react to
 *    internal events without introducing direct dependencies.
 *
 *  Author:  Dennis Kühnlein
 *  Version: v0.1
 *  Created: 08.12.2025
 *
 *  License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */


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