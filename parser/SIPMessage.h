#ifndef PROJECT_SIPX_SIPMESSAGE_H
#define PROJECT_SIPX_SIPMESSAGE_H

#include <string>
#include <vector>

enum class MessageType : uint8_t {
    UNKNOWN,
    REQUEST,
    RESPONSE
};
struct NameAddr {
    std::string scheme{};
    std::string user{};
    std::string host{};
    std::string uri_param{};
    std::string tag{};
    std::string params{};

    bool operator==(const NameAddr&) const = default;
};

struct Via {
    std::string transport{};
    std::string host{};
    std::string port{};
    std::string branch{};
    std::string params{};
};

struct CSeq {
    std::string number{};
    std::string method{};

    bool operator==(const CSeq&) const = default;
};

struct Contact {
    std::string scheme{};
    std::string user{};
    std::string host{};
    std::string port{};
    std::string transport{};
    std::string uri_params{};
    std::string expires{};
    std::string params{};
};

struct Header {
    std::string name{};
    std::string value{};
    bool operator==(const Header&) const = default;
};

struct SIPMessage {
    MessageType type{MessageType::UNKNOWN};
    bool is_complete{false};

    std::string method{};
    std::string request_uri{};
    std::string status_code{};
    std::string reason{};

    NameAddr                    from{};
    NameAddr                    to{};
    std::vector<Via>            via{};
    std::vector<std::string>    route{};
    std::vector<std::string>    record_route{};
    CSeq                        cseq{};
    std::vector<Contact>        contacts{};
    std::string                 call_id{};
    std::string                 content_length{};
    std::string                 content_type{};
    std::string                 max_forwards{};
    std::vector<std::string>    allow{};
    std::vector<std::string>    supported{};
    std::vector<std::string>    require{};

    std::vector<Header>         unknown_header{};
    std::vector<Header>         malformed_header{};
    std::vector<Header>         duplicate_header{};
    std::string                 body{};
};

#endif //PROJECT_SIPX_SIPMESSAGE_H