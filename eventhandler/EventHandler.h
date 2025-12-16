/*
*  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. It provides
 *  developers and DevOps engineers fine-grained control over SIP behavior,
 *  including non-RFC-compliant flows and corner-case simulations.
 *
 *  Description of EventHandler:
 *  ----------------------------
 *  EventHandler is the base interface for all components that want to receive
 *  events dispatched through the SIPx event system. Any module that needs to
 *  react to internal framework events must implement this class.
 *
 *  Main responsibilities:
 *    • Define the on_event(...) callback for handling dispatched Event objects
 *    • Provide a common interface for all event-reactive SIPx components
 *
 *  Internal structure:
 *    • Abstract class containing only a virtual destructor and the event callback
 *    • No state is stored; all behavior is implemented in derived classes
 *
 *  Purpose and usage:
 *    Modules register their EventHandler-derived objects at an EventDispatcher
 *    instance. Whenever the dispatcher broadcasts an Event, the on_event(...)
 *    method of each registered handler is invoked.
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


#ifndef PROJECT_SIPX_EVENTHANDLER_H
#define PROJECT_SIPX_EVENTHANDLER_H

#include "Event.h"

class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void on_event(Event& evt) = 0;
};

#endif //PROJECT_SIPX_EVENTHANDLER_H