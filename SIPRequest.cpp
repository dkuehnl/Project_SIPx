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
    auto start = std::chrono::high_resolution_clock::now();
    parse_request_line();
    parse_from();
    parse_to();
    parse_cseq();
    parse_contact();
    parse_pai();
    parse_ppi();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "[SIPRequest All] Execution-Time: " << duration.count() << "ms" << std::endl;


    /*

    ;*/

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

void SIPRequest::parse_cseq() {
    auto first_space = cseq_header.find(' ');
    auto line_end = cseq_header.find('\r');
    if (first_space == std::string_view::npos || line_end == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_cseq()]: Malformed CSeq header!" << std::endl;
        return;
    }

    cseq_nr = cseq_header.substr(0, first_space);
    cseq_typ = cseq_header.substr(first_space + 1, line_end - first_space - 1);
}

void SIPRequest::parse_contact() {
    auto contact = get_header("Contact");
    if (contact.empty()) {
        std::cerr << "[SIPRequest::parse_contact()]: No Contact-header found!" << std::endl;
        return;
    }
    auto uri_begin = contact.find("sip:");
    auto host_sep = contact.find('@', uri_begin);
    if (uri_begin == std::string_view::npos && host_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_contact()]: Malformed Contact header!" << std::endl;
        return;
    }
    auto port_sep = contact.find(':', host_sep);
    auto param_sep = contact.find(';', port_sep);
    auto header_end = contact.find('>', host_sep);
    auto line_end = contact.find('\r');

    contact_uri = contact.substr(uri_begin + 4, host_sep - (uri_begin + 4));
    contact_host = contact.substr(host_sep + 1, port_sep - host_sep - 1);
    contact_port = contact.substr(port_sep + 1, param_sep - port_sep - 1);
    contact_transport = contact.substr(param_sep + 1, header_end - param_sep - 1);
    contact_param = contact.substr(header_end + 1, line_end - header_end - 1);
}

void SIPRequest::parse_pai() {
    auto pai = get_header("P-Asserted-Identity");
    if (pai.empty()) {
        pai_uri = {};
        pai_host = {};
        return;
    }

    auto uri_begin = pai.find("sip:");
    if (uri_begin == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_pai()]: Malformed PAI header!" << std::endl;
        return;
    }
    auto host_sep = pai.find('@', uri_begin + 4);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_pai()]: Malformed PAI header!" << std::endl;
        return;
    }
    auto line_end = pai.find('>', host_sep + 1);
    if (line_end == std::string_view::npos) {
        line_end = pai.find('\r');
    }
    pai_uri = pai.substr(uri_begin + 4, host_sep - (uri_begin + 4));
    pai_host = pai.substr(host_sep + 1, line_end - host_sep - 1);
}

void SIPRequest::parse_ppi() {
    auto ppi = get_header("p-preferred-identity");
    if (ppi.empty()) {
        ppi_uri = {};
        ppi_host = {};
        return;
    }

    auto uri_begin = ppi.find("sip:");
    if (uri_begin == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_ppi()]: Malformed PPI header!" << std::endl;
        return;
    }
    auto host_sep = ppi.find('@', uri_begin + 4);
    if (host_sep == std::string_view::npos) {
        std::cerr << "[SIPRequest::parse_ppi()]: Malformed PPI header!" << std::endl;
        return;
    }
    auto line_end = ppi.find('>', host_sep + 1);
    if (line_end == std::string_view::npos) {
        line_end = ppi.find('\r');
    }
    ppi_uri = ppi.substr(uri_begin + 4, host_sep - (uri_begin + 4));
    ppi_host = ppi.substr(host_sep + 1, line_end - host_sep - 1);
}