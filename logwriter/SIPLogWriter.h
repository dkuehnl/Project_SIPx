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
#include <filesystem>
#include "../eventhandler/EventHandler.h"
#include "../eventhandler/EventDispatcher.h"

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

class SIPLogWriter : public EventHandler {
public:
    explicit SIPLogWriter(const std::string &filepath, EventDispatcher* dispatcher = nullptr);

    void write_log(const std::string_view msg);
    static std::string_view parse_error_code(ErrorCode code);

    void on_event(Event& evt) override;

private:
    bool check_prepare_filepath(const std::string& input);
    static bool can_write_to_dir(const std::filesystem::path& dir);
    static bool is_valid_path(const std::string& filepath);
    static bool dir_exists(const std::filesystem::path& dir);

    std::filesystem::path m_filepath;
    EventDispatcher* m_dispatcher = nullptr;
};


#endif //SIP_PARSER_SIPLOGWRITER_H