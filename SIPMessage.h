//
// Created by dkueh on 03.11.2025.
//

#ifndef SIP_PARSER_SIPMESSAGE_H
#define SIP_PARSER_SIPMESSAGE_H
#include <cstdint>
#include <string>
#include <unordered_map>


class SIPMessage {
public:
    explicit SIPMessage(std::string message, std::string source_ip = "0.0.0.0", std::uint16_t source_port = 5060);
    virtual ~SIPMessage() = default;

    std::string_view request_line() const { return m_request_line; };
    std::string_view from() const { return from_header; };
    std::string_view to() const { return to_header; };
    std::string_view cseq() const { return cseq_header; };
    std::string_view content_length() const { return content_length_header; };
    std::string_view get_header(std::string_view header_value) const;

    virtual void parse_message();

protected:
    std::string m_message;
    std::string m_source_ip;
    std::uint16_t m_source_port;

    std::string m_request_line;
    std::string_view from_header;
    std::string_view to_header;
    std::string_view cseq_header;
    std::string_view content_length_header;

    std::unordered_map<std::string_view, std::string_view> m_headers;
};



#endif //SIP_PARSER_SIPMESSAGE_H