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


#include "SIPMessage.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <utility>
#include <chrono>

const std::unordered_map<std::string_view, SIPMessage::HandlerFn>
SIPMessage::handlers = {
    { "Supported", &SIPMessage::handle_supported },
    { "Require", &SIPMessage::handle_required },
    { "Allow", &SIPMessage::handle_allow }
};

static std::string_view trim(std::string_view sv) {
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front())))
        sv.remove_prefix(1);
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back())))
        sv.remove_suffix(1);
    return sv;
}

std::vector<std::string_view> SIPMessage::parse_values(const std::vector<std::string_view>& list) {
    std::vector<std::string_view> final_values;

    for (std::string_view sv : list) {
        size_t start = 0;
        while (start < sv.size()) {
            size_t pos = sv.find(',', start);

            std::string_view token;
            if (pos == std::string_view::npos) {
                token = sv.substr(start);
                token = trim(token);
                if (!token.empty()) {
                    final_values.push_back(token);
                    break;
                }
            }
            token = sv.substr(start, pos - start);
            token = trim(token);
            if (!token.empty()) {
                final_values.push_back(token);
            }
            start = pos + 1;
        }
    }

    return final_values;
}

SIPMessage::SIPMessage(std::string message, std::string source_ip, std::uint16_t source_port)
    : m_message(std::move(message)), m_source_ip(std::move(source_ip)), m_source_port(source_port) {

    if (m_message.empty()) {
        throw std::invalid_argument("[SIPMessage.cpp] Message is empty");
    }
    if (m_source_ip.empty()) {
        throw std::invalid_argument("[SIPMessage.cpp] No Source-IP detected");
    }
    if (m_source_port == 0) {
        throw std::invalid_argument("[SIPMessage.cpp] Invalid Source-Port detected");
    }

    if (m_message.find("\r\n\r\n") == std::string::npos) {
        throw std::invalid_argument("[SIPMessage.cpp] Message does not end with \r\n");
    }


    auto start = std::chrono::high_resolution_clock::now();
    SIPMessage::parse_message();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Execution-Time: " << duration.count() << "ms" << std::endl;
}


void SIPMessage::parse_message() {
    const char* ptr = m_message.data();
    const char* end = ptr + m_message.size();

    const char* line_end = std::search(ptr, end, "\r\n", "\r\n" + 2);
    if (line_end != end) {
        m_request_line = std::string_view(ptr, line_end - ptr);
        ptr = line_end + 2;
    }

    while (ptr < end) {
        const char* next_line = std::search(ptr, end, "\r\n", "\r\n" + 2);
        if (next_line == ptr || next_line == end) break;

        const char* colon = std::find(ptr, next_line, ':');
        if (colon != next_line) {
            std::string_view name(ptr, colon - ptr);
            std::string_view value(colon + 1, next_line - colon);

            while (!value.empty() && value.front() == ' ') value.remove_prefix(1);

            auto cmp = [](std::string_view a, std::string_view b) {
                if (a.size() != b.size()) return false;
                for (size_t i = 0; i < a.size(); ++i) {
                    if (std::tolower(a[i]) != std::tolower(b[i])) return false;
                }
                return true;
            };

            if (cmp(name, "From")) from_header = value;
            else if (cmp(name, "To")) to_header = value;
            else if (cmp(name, "CSeq")) cseq_header = value;
            else {
                auto& vec = m_headers[name];
                vec.push_back(value);
            }
        }
        ptr = next_line + 2;
    }

    //Parse header completly:
    parse_from();
    parse_to();
    parse_cseq();
    parse_via();
    parse_contact();
    parse_content();
    parse_list_headers("Supported");
    parse_list_headers("Require");
    parse_list_headers("Allow");
    parse_session_expire();

    if (has_content) {
        const std::string_view content = m_message.data();
        auto sip_end = content.find("\r\n\r\n");
        sdp_content = content.substr(sip_end + 4);
        if (sdp_content.size() != content_length) {
            std::cerr << "[SIPMessage::parse_message()] Incomplete Content" << std::endl;
            return;
        }
    }
}

const std::vector<std::string_view>& SIPMessage::get_header(const std::string_view header_value) const {
    static const std::vector<std::string_view> empty;
    auto element = m_headers.find(header_value);
    if (element != m_headers.end()) {
        return element->second;
    }
    return empty;
}

void SIPMessage::parse_via() {
    const auto& vias = get_header("Via");
    if (vias.empty()) {
        std::cerr << "[SIPMessage parse_via()]: Malformed SIP-Message, no VIA-Headers" << std::endl;
        return;
    }

    for (const auto& via : vias) {
        auto transport_begin = via.find("SIP/2.0/");
        if (transport_begin == std::string_view::npos) {
            std::cerr << "[SIPMessage parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto uri_begin = via.find(' ', transport_begin + 3);
        if (uri_begin == std::string_view::npos) {
            std::cerr << "[SIPMessage parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto port_sep = via.find(':', uri_begin + 1);
        if (port_sep == std::string_view::npos) {
            std::cerr << "[SIPMessage parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto branch_begin = via.find("branch=");
        if (branch_begin == std::string_view::npos) {
            std::cerr << "[SIPMessage parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto line_end = via.find('\r', branch_begin);
        if (line_end == std::string_view::npos) {
            std::cerr << "[SIPMessage parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }

        auto transport = via.substr(transport_begin + 8, uri_begin - (transport_begin + 8));
        auto uri = via.substr(uri_begin + 1, port_sep - uri_begin - 1);
        auto port = via.substr(port_sep + 1, branch_begin - port_sep - 2);
        auto branch = via.substr(branch_begin + 7, line_end - (branch_begin + 7));

        via_list.emplace_back(
            SIP_VIA_Header{
                transport,
                uri,
                port,
                branch
            }
        );
    }
}

void SIPMessage::parse_contact() {
    const auto& contacts = get_header("Contact");
    if (contacts.size() > 1) {
        std::cerr << "[SIPMessage::parse_contact()]: More than one Contact found." << std::endl;
        return;
    }

    for (const auto& contact : contacts) {
        if (contact.empty()) {
            std::cerr << "[SIPMessage::parse_contact()]: No Contact-header found!" << std::endl;
            return;
        }
        auto uri_begin = contact.find("sip:");
        auto host_sep = contact.find('@', uri_begin);
        if (uri_begin == std::string_view::npos && host_sep == std::string_view::npos) {
            std::cerr << "[SIPMessage::parse_contact()]: Malformed Contact header!" << std::endl;
            return;
        }
        auto port_sep = contact.find(':', host_sep);
        auto param_sep = contact.find(';', port_sep);
        auto header_end = contact.find('>', host_sep);
        auto line_end = contact.find('\r');

        contact_uri = contact.substr(uri_begin + 4, host_sep - (uri_begin + 4));
        contact_host = contact.substr(host_sep + 1, port_sep - host_sep - 1);
        contact_port = contact.substr(port_sep + 1, param_sep - port_sep - 1);
        contact_transport = contact.substr(param_sep + 1, header_end - param_sep - 1);
        contact_param = contact.substr(header_end + 1, line_end - header_end - 1);
    }
}


void SIPMessage::parse_cseq() {
    auto first_space = cseq_header.find(' ');
    auto line_end = cseq_header.find('\r');
    if (first_space == std::string_view::npos || line_end == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_cseq()]: Malformed CSeq header!" << std::endl;
        return;
    }

    cseq_nr = cseq_header.substr(0, first_space);
    cseq_typ = cseq_header.substr(first_space + 1, line_end - first_space - 1);
}

void SIPMessage::parse_to() {
    auto first_sep = to_header.find("sip");
    if (first_sep == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto host_sep = to_header.find('@', first_sep + 1);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto line_end = to_header.find('\r', host_sep + 1);
    if (line_end == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto end_brack = to_header.find('>', host_sep + 1);
    auto last_sep = to_header.find(';', end_brack != std::string_view::npos ? end_brack + 1 : host_sep + 1);


    to_uri = to_header.substr(first_sep + 4, host_sep - (first_sep + 4));
    if (last_sep == std::string_view::npos) {
        to_tag = {};
        to_host = to_header.substr(host_sep + 1, end_brack != std::string_view::npos ? end_brack - host_sep - 1 : line_end - host_sep - 1);
    } else {
        to_host = to_header.substr(host_sep + 1, end_brack != std::string_view::npos ? end_brack - host_sep - 1 : last_sep - host_sep - 1);
        to_tag = to_header.substr(last_sep + 1, line_end - last_sep - 1);
    }
}

void SIPMessage::parse_from() {
    auto first_sep = from_header.find("sip");
    if (first_sep == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_from()]: Malformed URI header, no sip: found!" << std::endl;
        return;
    }
    auto host_sep = from_header.find('@', first_sep + 1);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_from()]: Malformed URI header, no @ found!" << std::endl;
        return;
    }
    auto end_brack = from_header.find('>', host_sep + 1);
    auto last_sep = from_header.find(';', end_brack != std::string_view::npos ? end_brack + 1 : host_sep + 1);
    if (last_sep == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_from()]: Malformed URI header, no end-limiter (; or >) found!" << std::endl;
        return;
    }
    auto line_end = from_header.find('\r', last_sep + 1);
    if (line_end == std::string_view::npos) {
        std::cerr << "[SIPMessage::parse_from()]: Malformed URI header, no line-end found!" << std::endl;
        return;
    }

    from_uri = from_header.substr(first_sep + 4, host_sep - (first_sep + 4) );
    if (end_brack != std::string_view::npos) {
        from_host = from_header.substr(host_sep + 1, end_brack - host_sep - 1);
    } else {
        from_host = from_header.substr(host_sep + 1, last_sep - host_sep - 1);
    }
    from_tag = from_header.substr(last_sep + 1, line_end - last_sep - 1);
}

void SIPMessage::parse_list_headers(const std::string& header_name) {
    const auto& list = get_header(header_name);
    if (list.empty()) {
        std::cerr << "[SIPMessage::parse_list_headers()]: No header with " << header_name << " found." << std::endl;
        return;
    }

    auto it = handlers.find(header_name);
    if (it == handlers.end()) {
        std::cerr << "[SIPMessage::parse_list_headers()]: No handler for header: " << header_name << std::endl;
        return;
    }

    auto handler = it->second;
    for (auto val : parse_values(list)) {
        (this->*handler)(val);
    }
}

void SIPMessage::parse_session_expire() {
    const auto& min_se_list = get_header("Min-SE");
    if (min_se_list.empty() || min_se_list.size() > 2) {
        std::cout << "[SIPMessage parse_session_expire()]: No Min-SE-Header or invalid" << std::endl;
    } else {
        for (const auto& value : min_se_list) {
            auto line_end = value.find('\r');
            auto temp_val = value.substr(0, line_end);
            uint16_t temp_int_val = 0;
            try {
                temp_int_val = static_cast<uint16_t>(std::stoi(std::string(temp_val)));
            } catch (const std::exception& e) {
                std::cerr << "[SIPMessage::parse_session_expire()]: Invalid Min-SE-value: " << e.what() << std::endl;
                return;
            }
            min_se = temp_int_val;
        }
    }

    const auto& session_expire = get_header("Session-Expires");
    if (session_expire.empty()) {
        std::cout << "[SIPMessage parse_session_expire()]: No Session-Expire" << std::endl;
        return;
    }

     for (const auto& value : session_expire) {
         auto refresher_start = value.find("refresher=");
         auto line_end = value.find('\r');

         if (line_end == std::string_view::npos) {
            std::cerr << "[SIPMessage parse_session_expire()]: No Line-End found" << std::endl;
            return;
         }

         std::string_view temp_timer;
         uint16_t temp_timer_val = 0;
         if (refresher_start == std::string_view::npos) {
             temp_timer = value.substr(0, line_end);
         } else {
             temp_timer = value.substr(0, refresher_start - 1);
             refresher = value.substr(refresher_start + 10, line_end - (refresher_start + 10));
         }

         try {
             temp_timer_val = static_cast<uint16_t>(std::stoi(std::string(temp_timer)));
         } catch (const std::exception& e) {
             std::cerr << "[SIPMessage::parse_session_expire()]: Invalid Refresh-Timer: " << e.what() << std::endl;
             return;
         }
         refresh_timer = temp_timer_val;
     }
}

void SIPMessage::parse_content() {
    const auto& list_content_length = get_header("Content-Length");
    if (list_content_length.size() != 1) {
        std::cerr << "[SIPMessage::parse_content()]: Malformed SIP-Message, no valid Content-Length-Header" << std::endl;
        return;
    }
    uint16_t tmp_cont_length = 0;
    try {
        tmp_cont_length = static_cast<uint16_t>(std::stoi(std::string(list_content_length[0])));
    } catch (const std::exception& e) {
        std::cerr << "[SIPMessage::parse_content()]: Invalid Content-Length: " << e.what() << std::endl;
        return;
    }
    content_length = tmp_cont_length;
    if (content_length <= 0) {
        has_content = false;
        return;
    }
    has_content = true;

    const auto& list_content_type = get_header("Content-Type");
    if (list_content_type.empty()) {
        std::cerr << "[SIPMessage::parse_content()]: Content-Type missing" << std::endl;
        return;
    }

    for (const auto& value : parse_values(list_content_type)) {
        if (value == "application/sdp") {
            content_type = value;
            return;
        }
    }
    std::cerr << "[SIPMessage::parse_content()]: No valid content-type found" << std::endl;
    return; //for future purpose (reason-causes are planned)
}