/*
 *  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. Its main goal
 *  is to give developers and DevOps engineers the ability to inspect,
 *  manipulate, and stress-test SIP/SDP/RTP flows in detail, including realistic
 *  failure and corner-case simulations.
 *
 *
 *  Description of SIPRequest:
 *  --------------------------
 *  The SIPRequest class represents all inbound SIP request types derived from
 *  the SIPMessage base class. It specializes the generic parsing logic by
 *  focusing on the request-line structure and request-specific headers such as
 *  P-Asserted-Identity (PAI) and P-Preferred-Identity (PPI).
 *
 *  Main responsibilities:
 *    • Parsing the SIP request line (method, request URI, host)
 *    • Converting the SIP method string into a strongly typed SIPMethod enum
 *    • Extracting request-specific identity headers (PAI / PPI)
 *    • Providing direct accessors for method, URI, host, and parsed identity lists
 *    • Extending the base parser with additional logic for INVITE/UPDATE
 *      (e.g., detecting and preparing SDP parsing)
 *
 *  Internal structure:
 *    • The request-line is processed via parse_request_line(), which extracts
 *      the method token, request URI, and host portion.
 *    • The SIPMethod enum is used to categorize the method efficiently and
 *      enables lightweight dispatching inside parse_message().
 *    • PAI and PPI headers are parsed into SIP_P_Header structures, each
 *      containing a URI and host extracted from the original SIP header.
 *    • Similar to SIPMessage, all values are stored as string_view references
 *      pointing directly into the original SIP message buffer for performance
 *      and zero-copy efficiency.
 *
 *  Behavior and parsing notes:
 *    • Malformed PAI/PPI headers generate log warnings but do not abort
 *      processing, ensuring that edge-case scenarios can still be tested.
 *    • Unknown SIP methods are classified as SIPMethod::UNKNOWN and handled
 *      gracefully without causing exceptions or termination.
 *
 *  Purpose and usage:
 *    SIPRequest provides a structured and efficient representation of
 *    SIP request messages such as INVITE, REGISTER, OPTIONS, UPDATE, etc.
 *    It acts as the request-specific extension layer over the generic
 *    SIPMessage parsing logic and is intended to be used in scenarios where
 *    request semantics (method, identity headers, URI/host analysis) matter.
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



#include "SIPRequest.h"
#include <iostream>
#include <utility>
#include <chrono>
#include <vector>

SIPRequest::SIPRequest(std::string message, std::string source_ip, uint16_t source_port)
    : SIPMessage(std::move(message), std::move(source_ip), source_port) {

    SIPRequest::parse_message();
}

SIPMethod SIPRequest::parse_method_enum(std::string_view method) {
    std::string upper(method);
    std::transform(upper.begin(), upper.end(), upper.begin(), toupper);

    if (upper == "INVITE") return SIPMethod::INVITE;
    if (upper == "ACK") return SIPMethod::ACK;
    if (upper == "BYE") return SIPMethod::BYE;
    if (upper == "REGISTER") return SIPMethod::REGISTER;
    if (upper == "OPTIONS") return SIPMethod::OPTIONS;
    if (upper == "CANCEL") return SIPMethod::CANCEL;
    if (upper == "UPDATE") return SIPMethod::UPDATE;
    if (upper == "PRACK") return SIPMethod::PRACK;

    return SIPMethod::UNKNOWN;
}

void SIPRequest::parse_message() {
    parse_request_line();

    switch (m_method_enum) {
        case SIPMethod::INVITE:
            parse_pai();
            parse_ppi();
            //detect SDP
            break;
        case SIPMethod::ACK:

            break;
        case SIPMethod::BYE:
            break;
        case SIPMethod::REGISTER:
            break;
        case SIPMethod::OPTIONS:
            break;
        case SIPMethod::CANCEL:
            break;
        case SIPMethod::UPDATE:
            //detect SDP
            break;
        case SIPMethod::PRACK:
            break;
        case SIPMethod::UNKNOWN:
        default:
            break;
    }

}

void SIPRequest::parse_request_line() {
    auto request_line = SIPMessage::request_line();
    auto first_space = request_line.find(' ');
    auto second_space = request_line.find(' ', first_space + 1);
    auto host_seperator = request_line.find('@', first_space + 1);

    if (first_space != std::string_view::npos && second_space != std::string_view::npos) {
        m_method = request_line.substr(0, first_space);
        m_method_enum = parse_method_enum(m_method);
        m_uri = request_line.substr(first_space + 1, host_seperator - first_space - 1);
        m_host = request_line.substr(host_seperator + 1, second_space - host_seperator - 1);
    }
}

void SIPRequest::parse_pai() {
    const auto& pais = get_header("P-Asserted-Identity");
    if (pais.size() == 0) {
        pai_list.emplace_back(SIP_P_Header{"", ""});
        return;
    }

    for (const auto& pai : pais) {
        auto uri_begin = pai.find("sip:");
        if (uri_begin == std::string_view::npos) {
            std::cerr << "[SIPRequest::parse_pai()]: Malformed PAI header!" << std::endl;
            return;
        }
        auto host_sep = pai.find('@', uri_begin + 4);
        if (host_sep == std::string_view::npos) {
            std::cerr << "[SIPRequest::parse_pai()]: Malformed PAI header!" << std::endl;
            return;
        }
        auto line_end = pai.find('>', host_sep + 1);
        if (line_end == std::string_view::npos) {
            line_end = pai.find('\r');
        }
        std::string_view uri = pai.substr(uri_begin + 4, host_sep - (uri_begin + 4));
        std::string_view host = pai.substr(host_sep + 1, line_end - host_sep - 1);

        pai_list.emplace_back(SIP_P_Header{uri, host});
    }
}

void SIPRequest::parse_ppi() {
    const auto& ppis = get_header("p-preferred-identity");
    if (ppis.size() == 0) {
        ppi_list.emplace_back(SIP_P_Header{"", ""});
        return;
    }

    for (const auto& ppi : ppis) {
        auto uri_begin = ppi.find("sip:");
        if (uri_begin == std::string_view::npos) {
            std::cerr << "[SIPRequest::parse_ppi()]: Malformed PPI header!" << std::endl;
            return;
        }
        auto host_sep = ppi.find('@', uri_begin + 4);
        if (host_sep == std::string_view::npos) {
            std::cerr << "[SIPRequest::parse_ppi()]: Malformed PPI header!" << std::endl;
            return;
        }
        auto line_end = ppi.find('>', host_sep + 1);
        if (line_end == std::string_view::npos) {
            line_end = ppi.find('\r');
        }
        std::string_view uri = ppi.substr(uri_begin + 4, host_sep - (uri_begin + 4));
        std::string_view host = ppi.substr(host_sep + 1, line_end - host_sep - 1);
        ppi_list.emplace_back(SIP_P_Header{uri, host});
    }
}