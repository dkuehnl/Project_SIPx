/*
 *  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. Its main goal
 *  is to give developers and DevOps engineers the ability to inspect,
 *  manipulate, and stress-test SIP/SDP/RTP flows in detail, including realistic
 *  failure and corner-case simulations.
 *
 *  Description of SIPParser:
 *  --------------------------
 *  The SIPParser is a wrapper-class for usage with e.g. the SIPxApp of this project.
 *  It provides an interface where you can put a SIP-Message into and get the correct
 *  message-type-object (SIPResponse or SIPRequest) back.
 *  The logging-function is optional and log-messages where only generated if a pointer to
 *  an SIPLogWriter-object is given on instantiation of the SIPParser.
 *
 *  Main responsibilities:
 *    • Deciding if the message is a SIP-response or -request
 *    • Instantiate based on that decision the correct SIPResponse or SIPRequest object
 *    • Automatically activate logging if a SIPLogWriter is provided
 *    • Count parsed packets with this instance
 *
 *  Internal structure:
 *    • Only SIPLogWriter-pointer and a counter is hold in this object
 *    • Parsed messages are given back to the caller and are not hold here
 *
 *  Purpose and usage:
 *    The only purpose is to give a common interface for parsing the messages so that
 *    the caller doesn't have to think about what message-type to instantiate.
 *
 *  Author:  Dennis Kühnlein
 *  Version: v0.1
 *  Created: 05.11.2025
 *  Updated: 25.11.2025
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
 *
 */


#ifndef PROJECT_SIPX_SIPPARSER_H
#define PROJECT_SIPX_SIPPARSER_H

#include <memory>

#include "SIPMessage.h"
#include "../eventhandler/EventHandler.h"
#include "../eventhandler/EventDispatcher.h"
#include "../logwriter/SIPLogWriter.h"

class SIPParser : public EventHandler {
public:
    explicit SIPParser(EventDispatcher& disp, SIPLogWriter* logger = nullptr);
    std::unique_ptr<SIPMessage> parse_message(std::string&& sip_message);
    [[nodiscard]] unsigned int get_paket_count() const { return m_paket_count; }

    void on_event(const Event& evt) override;

private:
    EventDispatcher& m_dispatcher;
    SIPLogWriter* m_logger;
    unsigned int m_paket_count = 0;

    static bool is_response(std::string_view sip_message);
};


#endif //PROJECT_SIPX_SIPPARSER_H