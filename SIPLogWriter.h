//
// Created by dkueh on 14.11.2025.
//

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
    SIPLogWriter(const std::string& filepath);
    ~SIPLogWriter();

    void write_log(std::string_view msg);
    void parse_error_code(ErrorCode code);
};


#endif //SIP_PARSER_SIPLOGWRITER_H