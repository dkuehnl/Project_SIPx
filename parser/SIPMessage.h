#ifndef PROJECT_SIPX_SIPMESSAGE_H
#define PROJECT_SIPX_SIPMESSAGE_H

#include <string>
#include <vector>

struct NameAddr {
    std::string scheme{};
    std::string user{};
    std::string host{};
    std::string uri_param{};
    std::string tag{};
    std::string params{};
};

struct Via {
    std::string transport{};
    std::string host_ip{};
    std::string port{};
    std::string branch{};
};

struct CSeq {
    std::string number{};
    std::string method{};
};

struct Contact {
    std::string uri{};
    std::string host{};
    std::string port{};
    std::string transport{};
    std::string expire{};
    std::vector<std::string> parameter{};
};

struct Header {
    std::string name{};
    std::string value{};
};

struct SIPMessage {
    bool is_request{false};
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
    std::string                 call_id{};
    std::string                 content_length{};
    std::string                 content_type{};
    std::string                 max_forwards{};
    std::string                 allow{};
    std::vector<std::string>    supported{};
    std::vector<std::string>    require{};

    std::string                 body{};
};

#endif //PROJECT_SIPX_SIPMESSAGE_H