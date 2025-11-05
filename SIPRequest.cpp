//
// Created by dkueh on 04.11.2025.
//


#include "SIPRequest.h"
#include <iostream>
#include <utility>
#include <chrono>

SIPRequest::SIPRequest(std::string message, std::string source_ip, uint16_t source_port)
    : SIPMessage(std::move(message), std::move(source_ip), source_port) {

    SIPRequest::parse_message();
}

void SIPRequest::parse_message() {
    parse_request_line();
    parse_from();
    auto start = std::chrono::high_resolution_clock::now();
    parse_to();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "[SIPRequest to] Execution-Time: " << duration.count() << "ms" << std::endl;


    /*parse_cseq();
    parse_contact();
    parse_pai();
    parse_ppi();*/

}

void SIPRequest::parse_request_line() {
    auto request_line = SIPMessage::request_line();
    auto first_space = request_line.find(' ');
    auto second_space = request_line.find(' ', first_space + 1);
    auto host_seperator = request_line.find('@', first_space + 1);

    if (first_space != std::string_view::npos && second_space != std::string_view::npos) {
        m_method = request_line.substr(0, first_space);
        m_uri = request_line.substr(first_space + 1, host_seperator - first_space - 1);
        m_host = request_line.substr(host_seperator + 1, second_space - host_seperator - 1);
    }
}

void SIPRequest::parse_from() {

    auto first_sep = from_header.find("sip");
    if (first_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
        return;
    }
    auto host_sep = from_header.find('@', first_sep + 1);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
        return;
    }
    auto end_brack = from_header.find('>', host_sep + 1);
    auto last_sep = from_header.find(';', end_brack != std::string_view::npos ? end_brack + 1 : host_sep + 1);
    if (last_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
        return;
    }
    auto line_end = from_header.find('\r', last_sep + 1);
    if (line_end == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_from()]: Malformed URI header!" << std::endl;
        return;
    }

    from_uri = from_header.substr(first_sep + 4, host_sep - (first_sep + 4) );
    if (end_brack != std::string_view::npos) {
        from_host = from_header.substr(host_sep + 1, end_brack - host_sep - 1);
    } else {
        from_host = from_header.substr(host_sep + 1, last_sep - host_sep - 1);
    }
    from_tag = from_header.substr(last_sep + 1, line_end - last_sep - 1);
}

void SIPRequest::parse_to() {
    auto first_sep = to_header.find("sip");
    if (first_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto host_sep = to_header.find('@', first_sep + 1);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto line_end = to_header.find('\r', host_sep + 1);
    if (line_end == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_to()]: Malformed URI header!" << std::endl;
        return;
    }
    auto end_brack = to_header.find('>', host_sep + 1);
    auto last_sep = to_header.find(';', end_brack != std::string_view::npos ? end_brack + 1 : host_sep + 1);


    to_uri = to_header.substr(first_sep + 4, host_sep - (first_sep + 4));
    if (last_sep == std::string_view::npos) {
        to_tag = {};
        to_host = to_header.substr(host_sep + 1, end_brack != std::string_view::npos ? end_brack - host_sep - 1 : line_end - host_sep - 1);
    } else {
        to_host = to_header.substr(host_sep + 1, end_brack != std::string_view::npos ? end_brack - host_sep - 1 : last_sep - host_sep - 1);
        to_tag = to_header.substr(last_sep + 1, line_end - last_sep - 1);
    }
}