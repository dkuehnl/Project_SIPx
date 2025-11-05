//
// Created by dkueh on 04.11.2025.
//

#ifndef SIP_PARSER_SIPREQUEST_H
#define SIP_PARSER_SIPREQUEST_H

#include "SIPMessage.h"

class SIPRequest : public SIPMessage {
public:
    explicit SIPRequest(std::string message, std::string source_ip = "0.0.0.0", uint16_t source_port = 5060);

    std::string_view method() const { return m_method; }
    std::string_view request_uri() const { return m_uri; }
    std::string_view request_host() const { return m_host; }
    std::string_view get_from_uri() const { return from_uri; }
    std::string_view get_from_host() const { return from_host; }
    std::string_view get_from_tag() const { return from_tag; }
    std::string_view get_to_uri() const { return to_uri; }
    std::string_view get_to_host() const { return to_host; }
    std::string_view get_to_tag() const { return to_tag; }

protected:
    void parse_message() override;

private:
    void parse_request_line();
    std::string_view m_method;
    std::string_view m_uri;
    std::string_view m_host;

    void parse_from();
    std::string_view from_uri;
    std::string_view from_host;
    std::string_view from_tag;

    void parse_to();
    std::string_view to_uri;
    std::string_view to_host;
    std::string_view to_tag;

    //void parse_cseq();
    std::string_view cseq_nr;
    std::string_view cseq_typ;

    //void parse_contact();
    std::string_view contact_header;
    std::string_view contact_uri;
    std::string_view contact_host;
    std::string_view contact_param;

    //void parse_pai();
    std::string_view pai_header;
    std::string_view pai_uri;
    std::string_view pai_host;

    //void parse_ppi();
    std::string_view ppi_header;
    std::string_view ppi_uri;
    std::string_view ppi_host;
};

#endif //SIP_PARSER_SIPREQUEST_H