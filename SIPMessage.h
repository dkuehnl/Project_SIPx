//
// Created by dkueh on 03.11.2025.
//

#ifndef SIP_PARSER_SIPMESSAGE_H
#define SIP_PARSER_SIPMESSAGE_H
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>


struct SIP_P_Header {
    std::string_view uri;
    std::string_view host;
};

struct SIP_VIA_Header {
    std::string_view transport;
    std::string_view uri;
    std::string_view port;
    std::string_view branch;
};


class SIPMessage {
public:
    explicit SIPMessage(std::string message, std::string source_ip = "0.0.0.0", std::uint16_t source_port = 5060);
    virtual ~SIPMessage() = default;

    std::string_view request_line() const { return m_request_line; };
    std::string_view from() const { return from_header; };
    std::string_view to() const { return to_header; };
    std::string_view cseq() const { return cseq_header; };
    std::string_view content_length() const { return content_length_header; };
    const std::vector<std::string_view>& get_header(std::string_view header_value) const;



protected:
    virtual void parse_message();
    std::string m_message;
    std::string m_source_ip;
    std::uint16_t m_source_port;

    std::string m_request_line;
    std::string_view from_header;
    std::string_view to_header;
    std::string_view cseq_header;
    std::string_view content_length_header;
    bool has_payload = false;

    std::unordered_map<std::string_view, std::vector<std::string_view>> m_headers;

    //specific parsing-function needed for base-classes:
    void parse_via();
    std::vector<SIP_VIA_Header> via_list;

    void parse_from();
    std::string_view from_uri;
    std::string_view from_host;
    std::string_view from_tag;

    void parse_to();
    std::string_view to_uri;
    std::string_view to_host;
    std::string_view to_tag;

    void parse_cseq();
    std::string_view cseq_nr;
    std::string_view cseq_typ;

    void parse_contact();
    std::string_view contact_uri;
    std::string_view contact_host;
    std::string_view contact_port;
    std::string_view contact_transport;
    std::string_view contact_param;
};



#endif //SIP_PARSER_SIPMESSAGE_H