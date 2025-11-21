//
// Created by dkueh on 21.11.2025.
//

#include "SIPParser.h"

SIPParser::SIPParser(SIPLogWriter* logger) {
    if (logger != nullptr) {
        m_logger = logger;
    }
}

bool SIPParser::is_response(const std::string_view sip_message) {
    const auto first_line_end = sip_message.find("\r\n");
    std::string_view first_line = sip_message.substr(0, first_line_end);
    if (first_line.starts_with("SIP/2.0")) {
        return false;
    }
    return true;
}


std::unique_ptr<SIPMessage> SIPParser::parse_message(std::string&& sip_message) {
    if (is_response(sip_message)) {
        return std::make_unique<SIPResponse>(std::move(sip_message), m_logger);
    } else {
        return std::make_unique<SIPRequest>(std::move(sip_message), m_logger);
    }
}
