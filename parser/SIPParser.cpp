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


#include "SIPParser.h"

#include "SIPRequest.h"
#include "SIPResponse.h"

SIPParser::SIPParser(EventDispatcher& disp, SIPLogWriter* logger)
    : m_dispatcher(disp) {
    if (logger != nullptr) {
        m_logger = logger;
    }

    m_dispatcher.register_listener(this);
}

void SIPParser::on_event(const Event& evt) {
    if (evt.type == EventType::RAW_MESSAGE_RECEIVED) {
        std::string* sip_raw = evt.raw_msg;
        auto parsed_message = parse_message(std::move(*sip_raw));

        Event e2;
        e2.type = EventType::MESSAGE_PARSED;
        e2.parsed_msg = parsed_message.release();
        e2.ownership_claimed = false;
        m_dispatcher.dispatch(e2);
    }
}

bool SIPParser::is_response(const std::string_view sip_message) {
    const auto first_line_end = sip_message.find("\r\n");
    if (const std::string_view first_line = sip_message.substr(0, first_line_end); first_line.starts_with("SIP/2.0")) {
        return false;
    }
    return true;
}


std::unique_ptr<SIPMessage> SIPParser::parse_message(std::string&& sip_message) {
    if (is_response(sip_message)) {
        m_paket_count++;
        return std::make_unique<SIPResponse>(std::move(sip_message), m_logger);
    }
    m_paket_count++;
    return std::make_unique<SIPRequest>(std::move(sip_message), m_logger);
}
