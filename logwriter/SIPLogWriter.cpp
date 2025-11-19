//
// Created by dkueh on 14.11.2025.
//

#include "SIPLogWriter.h"

#include <iostream>
#include <ostream>
#include <utility>

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

SIPLogWriter::SIPLogWriter(std::string filepath)
    : m_filepath(std::move(filepath)) {

}

void SIPLogWriter::write_log(const std::string_view msg) {
    std::cout << msg << std::endl;
}

std::string_view SIPLogWriter::parse_error_code(ErrorCode code) {
    return error_names[static_cast<size_t>(code)];
}
