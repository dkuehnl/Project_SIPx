//
// Created by dkueh on 03.11.2025.
//

#include "SIPMessage.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <utility>
#include <chrono>

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
        throw std::invalid_argument("[SIPMessage.cpp] Message does not end with \r\n\r");
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
            else if (cmp(name, "Content-Length")) content_length_header = value;
            else {
                auto& vec = m_headers[name];
                vec.push_back(value);
            }
        }
        ptr = next_line + 2;
    }

    if (content_length_header > 0) {
        has_payload = true;
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
        std::cerr << "[SIPRequest parse_via()]: Malformed SIP-Message, no VIA-Headers" << std::endl;
        return;
    }

    for (const auto& via : vias) {
        auto transport_begin = via.find("SIP/2.0/");
        if (transport_begin == std::string_view::npos) {
            std::cerr << "[SIPRequest parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto uri_begin = via.find(' ', transport_begin + 3);
        if (uri_begin == std::string_view::npos) {
            std::cerr << "[SIPRequest parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto port_sep = via.find(':', uri_begin + 1);
        if (port_sep == std::string_view::npos) {
            std::cerr << "[SIPRequest parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto branch_begin = via.find("branch=");
        if (branch_begin == std::string_view::npos) {
            std::cerr << "[SIPRequest parse_via()]: Malformed VIA-Header" << std::endl;
            return;
        }
        auto line_end = via.find('\r', branch_begin);
        if (line_end == std::string_view::npos) {
            std::cerr << "[SIPRequest parse_via()]: Malformed VIA-Header" << std::endl;
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
        std::cerr << "[SIPRequest::parse_contact()]: More than one Contact found." << std::endl;
        return;
    }

    for (const auto& contact : contacts) {
        if (contact.empty()) {
            std::cerr << "[SIPRequest::parse_contact()]: No Contact-header found!" << std::endl;
            return;
        }
        auto uri_begin = contact.find("sip:");
        auto host_sep = contact.find('@', uri_begin);
        if (uri_begin == std::string_view::npos && host_sep == std::string_view::npos) {
            std::cerr << "[SIPRequest::parse_contact()]: Malformed Contact header!" << std::endl;
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
        std::cerr << "[SIPRequest::parse_cseq()]: Malformed CSeq header!" << std::endl;
        return;
    }

    cseq_nr = cseq_header.substr(0, first_space);
    cseq_typ = cseq_header.substr(first_space + 1, line_end - first_space - 1);
}

void SIPMessage::parse_to() {
    auto first_sep = to_header.find("sip");
    if (first_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto host_sep = to_header.find('@', first_sep + 1);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto line_end = to_header.find('\r', host_sep + 1);
    if (line_end == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_to()]: Malformed URI header!" << std::endl;
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
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
        return;
    }
    auto host_sep = from_header.find('@', first_sep + 1);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
        return;
    }
    auto end_brack = from_header.find('>', host_sep + 1);
    auto last_sep = from_header.find(';', end_brack != std::string_view::npos ? end_brack + 1 : host_sep + 1);
    if (last_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
        return;
    }
    auto line_end = from_header.find('\r', last_sep + 1);
    if (line_end == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
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