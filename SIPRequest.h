//
// Created by dkueh on 04.11.2025.
//

#ifndef SIP_PARSER_SIPREQUEST_H
#define SIP_PARSER_SIPREQUEST_H

#include "SIPMessage.h"

enum class SIPMethod {
    INVITE,
    ACK,
    BYE,
    REGISTER,
    OPTIONS,
    CANCEL,
    UPDATE,
    PRACK,
    UNKNOWN
};

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
    std::string_view get_cseq_nr() const { return cseq_nr; }
    std::string_view get_cseq_typ() const { return cseq_typ; }
    std::string_view get_contact_uri() const { return contact_uri; }
    std::string_view get_contact_host() const { return contact_host; }
    std::string_view get_contact_port() const { return contact_port; }
    std::string_view get_contact_transport() const { return contact_transport; }
    std::string_view get_contact_param() const { return contact_param; }
    const std::vector<SIP_P_Header>& get_pai() const { return pai_list; }
    const std::vector<SIP_P_Header>& get_ppi() const { return ppi_list; }
    const std::vector<SIP_VIA_Header>& get_via() const { return via_list; }

protected:
    void parse_message() override;

private:
    SIPMethod parse_method_enum(std::string_view method) const;
    SIPMethod m_method_enum;

    void parse_request_line();
    std::string_view m_method;
    std::string_view m_uri;
    std::string_view m_host;

    void parse_pai();
    std::vector<SIP_P_Header> pai_list;
    void parse_ppi();
    std::vector<SIP_P_Header> ppi_list;
};

#endif //SIP_PARSER_SIPREQUEST_H