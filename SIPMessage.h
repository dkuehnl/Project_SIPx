/*
 *  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. Its main goal
 *  is to give developers and DevOps engineers the ability to inspect,
 *  manipulate, and stress-test SIP/SDP/RTP flows in detail, including realistic
 *  failure and corner-case simulations.
 *
 *  Description of SIPMessage:
 *  --------------------------
 *  The SIPMessage class is the foundational base for all SIP messages used in
 *  the project. It performs the complete parsing of a raw SIP message
 *  (headers + SDP) and provides structured access through a set of
 *  well-defined getter functions.
 *
 *  Main responsibilities:
 *    • Parsing the request line, core SIP headers (From/To/CSeq/etc.), and SDP
 *    • Extracting URIs, hosts, tags, and parameters
 *    • Evaluating list-type headers such as Supported, Require, and Allow
 *    • Handling Session-Timer headers (Min-SE / Session-Expires)
 *    • Decomposing VIA headers into transport/host/port/branch components
 *    • Providing a handler dispatcher that automatically maps specific headers
 *      to matching callback functions
 *
 *  Internal structure:
 *    • All headers are stored as string_view objects referencing the original
 *      SIP message buffer, avoiding unnecessary copies.
 *    • Parsing logic is split into clearly separated sub-functions
 *      (parse_via, parse_from, parse_to, parse_cseq, parse_contact, …).
 *    • Malformed headers produce log output but do not abort parsing, enabling
 *      realistic edge-case and failure testing.
 *
 *  Purpose and usage:
 *    This class serves as the skeleton for derived message types
 *    (INVITE, REGISTER, 200 OK, etc.) and provides the shared parsing and
 *    validation logic for all SIP message variants within the engine.
 *
 *  Author:  Dennis Kühnlein
 *  Version: v0.1
 *  Created: 05.11.2025
 *  Updated: 15.11.2025
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

#ifndef SIP_PARSER_SIPMESSAGE_H
#define SIP_PARSER_SIPMESSAGE_H
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>


struct SIP_P_Header {
    std::string_view uri;
    std::string_view host;
};

struct SIP_VIA_Header {
    std::string_view transport;
    std::string_view uri;
    std::string_view port;
    std::string_view branch;
};


class SIPMessage {
public:
    explicit SIPMessage(std::string message, std::string source_ip = "0.0.0.0", std::uint16_t source_port = 5060);
    virtual ~SIPMessage() = default;

    std::string_view request_line() const { return m_request_line; }
    std::string_view from() const { return from_header; }
    std::string_view to() const { return to_header; }
    std::string_view cseq() const { return cseq_header; }
    std::string_view sdp() const { return sdp_content; }
    const std::vector<std::string_view>& get_header(std::string_view header_value) const;

    std::string_view get_from_uri() const { return from_uri; }
    std::string_view get_from_host() const { return from_host; }
    std::string_view get_from_tag() const { return from_tag; }
    std::string_view get_to_uri() const { return to_uri; }
    std::string_view get_to_host() const { return to_host; }
    std::string_view get_to_tag() const { return to_tag; }
    std::string_view get_cseq_nr() const { return cseq_nr; }
    std::string_view get_cseq_typ() const { return cseq_typ; }
    std::string_view get_contact_uri() const { return contact_uri; }
    std::string_view get_contact_host() const { return contact_host; }
    std::string_view get_contact_port() const { return contact_port; }
    std::string_view get_contact_transport() const { return contact_transport; }
    std::string_view get_contact_param() const { return contact_param; }
    const std::vector<SIP_VIA_Header>& get_via() const { return via_list; }

    bool is_100rel_supported() const { return supported_100_rel; }
    bool is_timer_supported() const { return supported_timer; }
    bool is_100rel_required() const { return required_100_rel; }
    bool is_timer_required() const { return required_timer; }
    bool is_update_allowed() const { return update_allowed; }

    uint16_t get_min_se() const { return min_se; }
    uint16_t get_refresh_timer() const { return refresh_timer; }
    std::string_view get_refresher() const { return refresher; }

    bool is_content_attached() const { return has_content; }
    uint16_t get_content_length() const { return content_length; }
    std::string_view get_content_type() const { return content_type; }


protected:
    virtual void parse_message();
    std::string m_message;
    std::string m_source_ip;
    std::uint16_t m_source_port;

    std::string_view m_request_line;
    std::string_view from_header;
    std::string_view to_header;
    std::string_view cseq_header;
    std::string_view sdp_content;
    std::unordered_map<std::string_view, std::vector<std::string_view>> m_headers;

    //specific parsing-function needed for base-classes:
    void parse_via();
    std::vector<SIP_VIA_Header> via_list;

    void parse_from();
    std::string_view from_uri;
    std::string_view from_host;
    std::string_view from_tag;

    void parse_to();
    std::string_view to_uri;
    std::string_view to_host;
    std::string_view to_tag;

    void parse_cseq();
    std::string_view cseq_nr;
    std::string_view cseq_typ;

    void parse_contact();
    std::string_view contact_uri;
    std::string_view contact_host;
    std::string_view contact_port;
    std::string_view contact_transport;
    std::string_view contact_param;

    void parse_content();
    bool has_content = false;
    uint16_t content_length = 0;
    std::string_view content_type;

    void parse_list_headers(const std::string& header_name);
    bool supported_100_rel = false;
    bool supported_timer = false;
    bool required_100_rel = false;
    bool required_timer = false;
    bool update_allowed = false;

    //Parsing-Funktion für Supported, Required und Allow-Header
    static std::vector<std::string_view> parse_values(const std::vector<std::string_view>& list);
    void parse_session_expire();
    uint16_t min_se;
    uint16_t refresh_timer;
    std::string_view refresher;

    //Function-Dispatcher
    using HandlerFn = void (SIPMessage::*)(std::string_view);
    static const std::unordered_map<std::string_view, HandlerFn> handlers;

    void handle_supported(const std::string_view val) {
        if (val == "100rel") supported_100_rel = true;
        if (val == "timer") supported_timer = true;
    }
    void handle_required(const std::string_view val) {
        if (val == "100rel") required_100_rel = true;
        if (val == "timer") required_timer = true;
    }
    void handle_allow(const std::string_view val) {
        if (val == "UPDATE") update_allowed = true;
    }
};



#endif //SIP_PARSER_SIPMESSAGE_H