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

#ifndef SIP_PARSER_SIPREQUEST_H
#define SIP_PARSER_SIPREQUEST_H

#include "SIPMessage.h"
#include "SIPLogWriter.h"

enum class SIPMethod {
    INVITE,
    ACK,
    BYE,
    REGISTER,
    OPTIONS,
    CANCEL,
    UPDATE,
    PRACK,
    UNKNOWN
};

class SIPRequest : public SIPMessage {
public:
    explicit SIPRequest(std::string message, SIPLogWriter& logger, std::string source_ip = "0.0.0.0", uint16_t source_port = 5060);

    std::string_view method() const { return m_method; }
    std::string_view request_uri() const { return m_uri; }
    std::string_view request_host() const { return m_host; }
    const std::vector<SIP_P_Header>& get_pai() const { return pai_list; }
    const std::vector<SIP_P_Header>& get_ppi() const { return ppi_list; }

protected:
    bool parse_message() override;

private:
    static SIPMethod parse_method_enum(std::string_view method);
    SIPMethod m_method_enum;

    ErrorCode parse_request_line();
    std::string_view m_method;
    std::string_view m_uri;
    std::string_view m_host;

    ErrorCode parse_pai();
    std::vector<SIP_P_Header> pai_list;
    ErrorCode parse_ppi();
    std::vector<SIP_P_Header> ppi_list;
};

#endif //SIP_PARSER_SIPREQUEST_H