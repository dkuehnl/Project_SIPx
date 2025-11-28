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

#include "SIPLogWriter.h"


#include <fstream>
#include <iostream>
#include <ostream>

static constexpr std::string_view error_names[] = {
    "OK",
    "BAD_REQUEST",
    "FORBIDDEN",
    "NOT_FOUND",
    "METHOD_NOT_ALLOWED",
    "NOT_ACCEPTABLE",
    "REQUEST_TIMEOUT",
    "LENGTH_REQUIRED",
    "UNSUPPORTED_MEDIA_TYPE",
    "UNSUPPORTED_URI_SCHEME",
    "BAD_EXTENSION",
    "SESSION_INTERVAL_TOO_SMALL",
    "INTERVAL_TOO_BRIEF",
    "SERVER_INTERNAL_ERROR",
    "NOT_IMPLEMENTED",
    "SERVICE_UNAVAILABLE"
};

SIPLogWriter::SIPLogWriter(const std::string &filepath, EventDispatcher* dispatcher) : m_dispatcher(dispatcher){
    if (m_dispatcher != nullptr) {
        m_dispatcher->register_listener(this);
    }

    if (!check_prepare_filepath(filepath)) {
        throw std::runtime_error("Invalid filepath");
    }
}

void SIPLogWriter::on_event(const Event &evt) {
    if (evt.type == EventType::LOG_MESSAGE) {
        write_log(evt.log_message);
    }
}


bool SIPLogWriter::check_prepare_filepath(const std::string &input) {
    if (!is_valid_path(input)) {
        return false;
    }

    const std::filesystem::path p(input);
    const std::filesystem::path dir = p.has_filename() ? p.parent_path() : p;

    if (dir.empty()) {
        return false;
    }
    if (!dir_exists(dir)) {
        return false;
    }
    if (!can_write_to_dir(dir)) {
        return false;
    }

    m_filepath = dir;
    return true;
}

bool SIPLogWriter::is_valid_path(const std::string &filepath) {
    std::error_code ec;
    std::filesystem::path p(filepath);

    auto st = std::filesystem::status(p, ec);
    return !ec;
}

bool SIPLogWriter::dir_exists(const std::filesystem::path &dir) {
    std::error_code ec;
    return std::filesystem::exists(dir, ec) && std::filesystem::is_directory(dir, ec);
}

bool SIPLogWriter::can_write_to_dir(const std::filesystem::path& dir) {
    const std::filesystem::path test_file = dir / ".__write_test__.tmp";
    std::ofstream ofs(test_file.string(), std::ios::out | std::ios::app);

    if (!ofs.is_open()) {
        return false;
    }

    ofs.close();
    std::filesystem::remove(test_file);
    return true;
}

void SIPLogWriter::write_log(const std::string_view msg) {
    std::cout << msg << std::endl;
}

std::string_view SIPLogWriter::parse_error_code(ErrorCode code) {
    return error_names[static_cast<size_t>(code)];
}
