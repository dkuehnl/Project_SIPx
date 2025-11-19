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
 *  Description of SIPLogWriter:
 *  --------------------------
 *  The SIPLogWriter is responsible to provide the error-codes used by the other
 *  parts of this software.
 *  It is also responsible to log all errors into a specific file which have to be
 *  defined while initiating an instance. Additionally, there is a helper-function to
 *  map the ENUM-values to the appropriate error-codes.
 *
 *  Main responsibilities:
 *    • Receive log-messages from other objects
 *    • Log these log-messages to a defined file-location
 *    • Mapping ENUM-values to printable error-codes
 *
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


#ifndef SIP_PARSER_SIPLOGWRITER_H
#define SIP_PARSER_SIPLOGWRITER_H
#include <string>

enum class ErrorCode {
    OK,
    BAD_REQUEST,
    FORBIDDEN,
    NOT_FOUND,
    METHOD_NOT_ALLOWED,
    NOT_ACCEPTABLE,
    REQUEST_TIMEOUT,
    LENGTH_REQUIRED,
    UNSUPPORTED_MEDIA_TYPE,
    UNSUPPORTED_URI_SCHEME,
    BAD_EXTENSION,
    SESSION_INTERVAL_TOO_SMALL,
    INTERVAL_TOO_BRIEF,
    SERVER_INTERNAL_ERROR,
    NOT_IMPLEMENTED,
    SERVICE_UNAVAILABLE
};

class SIPLogWriter {
public:
    explicit SIPLogWriter(std::string filepath);

    void write_log(std::string_view msg);
    static std::string_view parse_error_code(ErrorCode code);

private:
    std::string m_filepath;
};


#endif //SIP_PARSER_SIPLOGWRITER_H