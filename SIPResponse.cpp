//
// Created by dkueh on 04.11.2025.
//

#include "SIPResponse.h"

SIPResponse::SIPResponse(std::string message, std::string source_ip, uint16_t source_port)
    : SIPMessage(std::move(message), std::move(source_ip), source_port) {

    SIPResponse::parse_message();
}

void SIPResponse::parse_message() {
    parse_request_line();
    parse_from();
    parse_to();
    parse_cseq();
    parse_via();
    parse_contact();

    if (has_payload) {
        //parse sdp
    }
}

void parse_request_line() {

}

void parse_supported() {

}

void parse_required() {

}

void parse_allow() {

}

void parse_session_expire() {

}