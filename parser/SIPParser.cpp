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

#include <iostream>

#include "SIPRequest.h"
#include "SIPResponse.h"
#include "../utils/sip_utils.h"

namespace {
    constexpr std::string_view trim(std::string_view s) noexcept {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
        return s;
    }

    constexpr char ascii_lower(char c) noexcept {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
    }

    constexpr bool iequals(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (ascii_lower(a[i]) != ascii_lower(b[i])) return false;
        }

        return true;
    }

    std::size_t find_closing_quote(std::string_view s) noexcept {
        for (std::size_t i = 1; i < s.size(); ++i) {
            if (s[i] == '\\') { ++i; continue; }
            if (s[i] == '"') return i;
        }

        return std::string_view::npos;
    }

    void parse_uri(std::string_view uri, NameAddr& out) {
        const auto colon = uri.find(':');
        if (colon == std::string_view::npos) {
            //TODO: Log-Message
            return;
        }
        out.scheme = uri.substr(0, colon);
        uri.remove_prefix(colon + 1);

        if (const auto semi = uri.find(';'); semi != std::string_view::npos) {
            out.uri_param = uri.substr(semi + 1);
            uri = uri.substr(0, semi);
        }

        if (iequals(out.scheme, "tel")) {
            out.user = uri;
            return;
        }

        if (const auto at = uri.find('@'); at != std::string_view::npos) {
            out.user = uri.substr(0, at);
            uri.remove_prefix(at + 1);
        }

        if (!uri.empty() && uri.front() == '[') {
            const auto rb = uri.find(']');
            if (rb == std::string_view::npos) {
                //TODO: Logging
                return;
            }
            out.host = uri.substr(0, rb + 1);
        } else {
            out.host = uri.substr(0, uri.find(':'));
        }
    }

    void parse_header_params(std::string_view params, NameAddr& out) {
        params = trim(params);
        if (!params.empty() && params.front() == ';') params.remove_prefix(1);

        while (!params.empty()) {
            const auto semi = params.find(';');
            const auto param = trim(params.substr(0, semi));
            params.remove_prefix(semi == std::string_view::npos ? params.size() : semi + 1);

            const auto eq = param.find('=');
            if (eq != std::string_view::npos && iequals(trim(param.substr(0, eq)), "tag")) {
                out.tag = trim(param.substr(eq + 1));
                continue;
            }

            if (!out.params.empty()) out.params += ';';
            out.params += params;
        }
    }

    void parse_name_addr(std::string_view value, NameAddr& out) {
        std::size_t search_from = 0;

        if (!value.empty() && value.front() == '"') {
            const auto closing = find_closing_quote(value);
            if (closing == std::string_view::npos) {
                //TODO: Logging
                return;
            }

            search_from = closing + 1;
        }

        std::string_view addr;
        std::string_view params;

        if (const auto lt = value.find('<', search_from); lt != std::string_view::npos) {
            const auto gt = value.find('>', lt + 1);
            if (gt == std::string_view::npos) {
                //TODO: logging
                return;
            }
            addr = value.substr(lt + 1, gt - lt - 1);
            params = value.substr(gt + 1);
        } else {
            const auto semi = value.find(';');
            addr = value.substr(0, semi);
            params = (semi == std::string_view::npos) ? std::string_view{} : value.substr(semi);
        }

        parse_uri(trim(addr), out);
        parse_header_params(params, out);
    }

    void parse_call_id(std::string_view value, SIPMessage& msg) {
        msg.call_id = std::string{value};
    }

    void parse_content_length(std::string_view value, SIPMessage& msg) {
        msg.content_length = std::string{value};
    }

    void parse_content_type(std::string_view value, SIPMessage& msg) {
        msg.content_type = std::string{value};
    }

    void parse_max_forward(std::string_view value, SIPMessage& msg) {
        msg.max_forwards = std::string{value};
    }

    void parse_allow(std::string_view value, SIPMessage& msg) {
        msg.allow = std::string{value};
    }

    void parse_from(std::string_view value, SIPMessage& msg) {
        parse_name_addr(value, msg.from);
    }

    void parse_to(std::string_view value, SIPMessage& msg) {
        parse_name_addr(value, msg.to);
    }

    void parse_cseq(std::string_view value, SIPMessage& msg) {
        const auto sep = value.find(' ');
        if (sep == std::string_view::npos) {
            //TODO Logging
            return;
        }

        msg.cseq.number = trim(value.substr(0, sep));
        msg.cseq.method = trim(value.substr(sep + 1));
    }

    void parse_option_tags(std::string_view value, std::vector<std::string>& out) {
        while (!value.empty()) {
            const auto comma = value.find(',');
            const auto tag = trim(value.substr(0, comma));
            value.remove_prefix(comma == std::string_view::npos ? value.size() : comma + 1);

            if (!tag.empty()) out.emplace_back(tag);
        }
    }

    void parse_supported(std::string_view value, SIPMessage& msg) {
        parse_option_tags(value, msg.supported);
    }

    void parse_required(std::string_view value, SIPMessage& msg) {
        parse_option_tags(value, msg.require);
    }

    using Handler = void (*)(std::string_view value, SIPMessage& msg);

    struct KnownHeader {
        std::string_view name;
        std::string_view compact;
        Handler handler;
    };

    constexpr KnownHeader kKnownHeaders[] = {
        {"Call-ID", "i", parse_call_id},
        {"Content-Length", "l", parse_content_length},
        {"Content-Type", "c", parse_content_type},
        {"Max-Forwards", "", parse_max_forward},
        {"Allow", "", parse_allow},
        {"From", "f", parse_from},
        {"To", "t", parse_to},
        {"CSeq", "", parse_cseq},
        {"Supported", "k", parse_supported},
        {"Require", "", parse_required}
    };

    bool dispatch(std::string_view header_name, std::string_view value, SIPMessage& msg) {
        for (const auto& known : kKnownHeaders) {
            if (iequals(header_name, known.name) || (!known.compact.empty() && iequals(header_name, known.compact))) {
                known.handler(value, msg);
                return true;
            }
        }

        return false;
    }

}

SIPParser::SIPParser(EventDispatcher* disp, SIPLogWriter* logger)
    : m_dispatcher(disp), m_logger(logger) {
    if (disp != nullptr) {
        m_dispatcher->register_listener(this);
    }
}

void SIPParser::on_event(Event& evt) {
    if (evt.type == EventType::RAW_MESSAGE_RECEIVED) {
        std::string* sip_raw = evt.raw_msg;
        auto parsed_message = parse_message(std::move(*sip_raw));

        Event e2;
        e2.type = EventType::MESSAGE_PARSED;
        //e2.parsed_msg = std::move(parsed_message);
        e2.ownership_claimed = false;
        m_dispatcher->dispatch(e2);
    } else if (evt.type == EventType::TIME_TO_REGISTER) {
        std::cout << "Event funktioniert" << std::endl;
    }
}

bool SIPParser::parse_rline(std::string_view sip_message, SIPMessage& msg) {
    const auto request_line_end = sip_message.find("\r\n");
    const auto start_line = sip_message.substr(0, request_line_end);
    const auto sp1 = start_line.find(' ');

    if (start_line.starts_with("SIP/2.0 ")) {
        msg.is_request = false;
        const auto sp2 = start_line.find(' ', sp1 + 1);
        msg.status_code = start_line.substr(sp1 + 1, sp2 - sp1 - 1);
        msg.reason = (sp2 == std::string_view::npos) ? std::string_view{} : start_line.substr(sp2 + 1);

        return true;
    }

    if (start_line.ends_with(" SIP/2.0")) {
        msg.is_request = true;
        const auto sp2 = start_line.rfind(' ');
        msg.method = start_line.substr(0, sp1);
        msg.request_uri = start_line.substr(sp1 + 1, sp2 - sp1 - 1);

        return true;
    }

    return false;
}


SIPMessage SIPParser::parse_message(std::string_view sip_message) {
    SIPMessage message{};

    const auto message_end = sip_message.find("\r\n\r\n");
    const auto request_line_end = sip_message.find("\r\n");
    message.is_complete = message_end != std::string_view::npos && request_line_end > 0;

    if (!parse_rline(sip_message, message)) {

    }

    std::string_view sip_headers = sip_message.substr(request_line_end + 2, message_end - request_line_end - 2);

    while (!sip_headers.empty()) {
        const auto eol = sip_headers.find("\r\n");
        const auto line = sip_headers.substr(0, eol);
        sip_headers.remove_prefix(eol == std::string_view::npos ? sip_headers.size() : eol + 2);

        const auto colon = line.find(':');
        if (colon == std::string_view::npos) continue;

        const auto header_name = trim(line.substr(0, colon));
        const auto plain_value = trim(line.substr(colon + 1));

        if (!dispatch(header_name, plain_value, message)) {

        }
    }

    return message;
}

