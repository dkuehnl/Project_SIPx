/*
 *  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. Its main goal
 *  is to give developers and DevOps engineers the ability to inspect,
 *  manipulate, and stress-test SIP/SDP/RTP flows in detail, including realistic
 *  failure and corner-case simulations.
 *
 *  Description of SIPResponse:
 *  ---------------------------
 *  The SIPResponse class represents all SIP response messages derived from
 *  the SIPMessage base class. It specializes the generic SIPMessage parsing
 *  logic to extract the response code and reason phrase from the response
 *  start-line.
 *
 *  Main responsibilities:
 *    • Parsing the response start-line to extract the SIP version, response code,
 *      and response type (reason phrase)
 *    • Providing accessors for response code (numeric) and response type (string)
 *    • Serving as a foundation for handling specific response-related headers
 *      in derived classes, if needed
 *
 *  Internal structure:
 *    • The response start-line is parsed in parse_request_line(), which identifies
 *      the numeric response code and the textual reason phrase.
 *    • The numeric code is stored as uint16_t, and the reason phrase is stored
 *      as a string_view referencing the original SIP message buffer.
 *    • All parsing is performed safely, with malformed start-lines producing
 *      log warnings but not terminating execution, allowing robust testing of
 *      edge-case responses.
 *
 *  Behavior and parsing notes:
 *    • Malformed or unexpected response lines generate log output.
 *    • The class does not throw exceptions for invalid response codes; instead,
 *      it logs the issue and continues parsing where possible.
 *
 *  Purpose and usage:
 *    SIPResponse provides a structured and efficient representation of SIP
 *    response messages such as 200 OK, 180 Ringing, 404 Not Found, etc.
 *    It acts as the response-specific extension layer over the generic
 *    SIPMessage parsing logic and is intended for use in testing,
 *    simulation, or processing of SIP responses.
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

#ifndef SIP_PARSER_SIPRESPONSE_H
#define SIP_PARSER_SIPRESPONSE_H

#include "SIPMessage.h"


class SIPResponse : public SIPMessage {
public:
    explicit SIPResponse(std::string message, SIPLogWriter* logger = nullptr, EventDispatcher* dispatcher = nullptr);

    uint16_t get_response_code() const { return response_code; }
    std::string_view get_response_type() const { return response_type; }

protected:
    bool parse_message() override;

private:
    ErrorCode parse_request_line();
    std::string_view response_type;
    uint16_t response_code;
};

#endif //SIP_PARSER_SIPRESPONSE_H