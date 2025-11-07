//
// Created by dkueh on 05.11.2025.
//

#ifndef SIP_PARSER_SIPRESPONSE_H
#define SIP_PARSER_SIPRESPONSE_H

#include "SIPMessage.h"


class SIPResponse : public SIPMessage {
public:
    explicit SIPResponse(std::string message, std::string source_ip = "0.0.0.0", uint16_t source_port = 0);

protected:
    void parse_message() override;

private:
    void parse_request_line();
    std::string_view response_type;
    uint16_t response_code;

    void parse_supported();
    std::vector<std::string_view> supported_list;

    void parse_requried();
    std::vector<std::string_view> required_list;

    void parse_allow();
    std::vector<std::string_view> allow_list;

    void parse_session_expire();
    std::string_view refresh_timer;
    std::string_view refresher;
};

#endif //SIP_PARSER_SIPRESPONSE_H