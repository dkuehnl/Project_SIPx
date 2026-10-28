//
// Created by dkueh on 05.11.2025.
//

#include "SIPParser.h"

#include <iostream>
#include <algorithm>
#include <optional>
#include <bitset>


namespace {
    struct UriParts {
        std::string_view scheme{};
        std::string_view user{};
        std::string_view host{};
        std::string_view port{};
        std::string_view params{};
    };

    struct NameAddrParts {
        std::string_view addr{};
        std::string_view params{};
    };

    struct HostPort {
        std::string_view host{};
        std::string_view port{};
    };

    constexpr std::string_view trim(std::string_view s) noexcept {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
        return s;
    }

    constexpr char ascii_lower(char c) noexcept {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
    }

    constexpr bool iequals(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (ascii_lower(a[i]) != ascii_lower(b[i])) return false;
        }

        return true;
    }

    std::size_t find_closing_quote(std::string_view s) noexcept {
        for (std::size_t i = 1; i < s.size(); ++i) {
            if (s[i] == '\\') { ++i; continue; }
            if (s[i] == '"') return i;
        }

        return std::string_view::npos;
    }

    std::optional<UriParts> split_uri(std::string_view uri) {
        UriParts p;

        const auto colon = uri.find(':');
        if (colon == std::string_view::npos) return std::nullopt;
        p.scheme = uri.substr(0, colon);
        uri.remove_prefix(colon + 1);

        if (const auto semi = uri.find(';'); semi != std::string_view::npos) {
            p.params = uri.substr(semi + 1);
            uri = uri.substr(0, semi);
        }

        if (iequals(p.scheme, "tel")) {
            p.user = uri;
            return p;
        }

        if (const auto at = uri.find('@'); at != std::string_view::npos) {
            p.user = uri.substr(0, at);
            uri.remove_prefix(at + 1);
        }

        std::size_t host_end = 0;
        if (!uri.empty() && uri.front() == '[') {
            const auto rb = uri.find(']');
            if (rb == std::string_view::npos) return std::nullopt;
            host_end = rb + 1;
        } else {
            host_end = std::min(uri.find(':'), uri.size());
        }

        p.host = uri.substr(0, host_end);
        if (host_end < uri.size() && uri[host_end] == ':') {
            p.port = uri.substr(host_end + 1);
        }

        return p;
    }

    std::optional<NameAddrParts> split_name_addr(std::string_view value) {
        std::size_t search_from = 0;
        if (!value.empty() && value.front() == '"') {
            const auto closing = find_closing_quote(value);
            if (closing == std::string_view::npos) return std::nullopt;
            search_from = closing + 1;
        }

        if (const auto lt = value.find('<', search_from); lt != std::string_view::npos) {
            const auto gt = value.find('>', lt + 1);
            if (gt == std::string_view::npos) return std::nullopt;

            return NameAddrParts{trim(value.substr(lt + 1, gt - lt - 1)), value.substr(gt + 1)};
        }

        const auto semi = value.find(';');
        return NameAddrParts{trim(value.substr(0, semi)),
            semi == std::string_view::npos
            ? std::string_view{}
            : value.substr(semi) };
    }

    std::string take_param(std::string_view params, std::string_view key, std::string& value) {
        std::string rest;

        params = trim(params);
        if (!params.empty() && params.front() == ';') params.remove_prefix(1);

        while (!params.empty()) {
            const auto semi = params.find(';');
            const auto param = trim(params.substr(0, semi));
            params.remove_prefix(semi == std::string_view::npos ? params.size() : semi + 1);

            if (param.empty()) continue;

            const auto eq = param.find('=');
            if (eq != std::string_view::npos && iequals(trim(param.substr(0, eq)), key)) {
                value = trim(param.substr(eq + 1));
                continue;
            }

            if (!rest.empty()) rest += ';';
            rest += param;
         }
        return rest;
    }

    std::string_view next_list_item(std::string_view& rest) {
        bool in_quotes{false};
        bool in_angle{false};

        for (std::size_t i = 0; i < rest.size(); ++i) {
            const char c = rest[i];

            if (in_quotes) {
                if (c == '\\') ++i;
                else if (c == '"') in_quotes = false;
                continue;
            }

            if (c == '"') in_quotes = true;
            else if (c == '<') in_angle = true;
            else if (c == '>') in_angle = false;
            else if (c == ',' && !in_angle) {
                const auto item = trim(rest.substr(0, i));
                rest.remove_prefix(i + 1);
                return item;
            }
        }

        const auto item = trim(rest);
        rest = {};
        return item;
    }

    void add_malformed(SIPMessage& msg, std::string_view name, std::string_view value) {
        msg.malformed_header.push_back({std::string{name}, std::string{value}});
    }

    bool parse_name_addr(std::string_view value, NameAddr& out) {
        const auto na = split_name_addr(value);
        if (!na) {
            //TODO: Logging
            return false;
        }

        const auto uri = split_uri(na->addr);
        if (!uri) {
            //TODO: Logging
            return false;
        }

        out.scheme  = uri->scheme;
        out.user    = uri->user;
        out.host    = uri->host;
        out.uri_param = uri->params;
        out.params  = take_param(na->params, "tag", out.tag);

        return true;
    }

    std::optional<HostPort> split_hostport(std::string_view value) {
        std::size_t host_end = 0;
        if (!value.empty() && value.front() == '[') {
            const auto rb = value.find(']');
            if (rb == std::string_view::npos) return std::nullopt;
            host_end = rb + 1;
        } else {
            host_end = std::min(value.find(':'), value.size());
        }

        HostPort hp{value.substr(0, host_end), {}};
        if (host_end < value.size() && value[host_end] == ':') hp.port = value.substr(host_end + 1);

        return hp;
    }

    void parse_option_tags(std::string_view value, std::vector<std::string>& out) {
        while (!value.empty()) {
            const auto comma = value.find(',');
            const auto tag = trim(value.substr(0, comma));
            value.remove_prefix(comma == std::string_view::npos ? value.size() : comma + 1);

            if (!tag.empty()) out.emplace_back(tag);
        }
    }

    void parse_call_id(std::string_view, std::string_view value, SIPMessage& msg) {
        msg.call_id = std::string{value};
    }

    void parse_content_length(std::string_view, std::string_view value, SIPMessage& msg) {
        msg.content_length = std::string{value};
    }

    void parse_content_type(std::string_view, std::string_view value, SIPMessage& msg) {
        msg.content_type = std::string{value};
    }

    void parse_max_forwards(std::string_view, std::string_view value, SIPMessage& msg) {
        msg.max_forwards = std::string{value};
    }

    void parse_allow(std::string_view, std::string_view value, SIPMessage& msg) {
        parse_option_tags(value, msg.allow);
    }

    void parse_from(std::string_view name, std::string_view value, SIPMessage& msg) {
        if (!parse_name_addr(value, msg.from)) add_malformed(msg, name, value);
    }

    void parse_to(std::string_view name, std::string_view value, SIPMessage& msg) {
        if (!parse_name_addr(value, msg.to)) add_malformed(msg, name, value);
    }

    void parse_cseq(std::string_view name, std::string_view value, SIPMessage& msg) {
        const auto sep = value.find(' ');
        if (sep == std::string_view::npos) {
            add_malformed(msg, name, value);
            //TODO Logging
            return;
        }

        msg.cseq.number = trim(value.substr(0, sep));
        msg.cseq.method = trim(value.substr(sep + 1));
    }

    void parse_supported(std::string_view, std::string_view value, SIPMessage& msg) {
        parse_option_tags(value, msg.supported);
    }

    void parse_require(std::string_view, std::string_view value, SIPMessage& msg) {
        parse_option_tags(value, msg.require);
    }

    void parse_contact(std::string_view name, std::string_view value, SIPMessage& msg) {
        while (!value.empty()) {
            const auto item = next_list_item(value);
            if (item.empty()) continue;

            const auto na = split_name_addr(item);
            const auto uri = na ? split_uri(na->addr) : std::nullopt;
            if (!uri) {
                add_malformed(msg, name, item);
                //TODO: Logging
                continue;
            }

            Contact& c  = msg.contacts.emplace_back();
            c.scheme    = uri->scheme;
            c.user      = uri->user;
            c.host      = uri->host;
            c.port      = uri->port;
            c.uri_params = take_param(uri->params, "transport", c.transport);
            c.params    = take_param(na->params, "expires", c.expires);
        }
    }

    void parse_via(std::string_view name, std::string_view value, SIPMessage& msg) {
        while (!value.empty()) {
            const auto item = next_list_item(value);
            if (item.empty()) continue;

            const auto first_ws = item.find(' ');
            if (first_ws == std::string_view::npos) {
                add_malformed(msg, name, item);
                //TODO: Logging
                continue;
            }

            const auto rest = trim(item.substr(first_ws + 1));
            const auto sep = rest.find(';');

            const auto hp = split_hostport(trim(rest.substr(0, sep)));
            if (!hp) {
                add_malformed(msg, name, item);
                //TODO: Logging
                continue;
            }

            auto& [transport, host, port, branch, params] = msg.via.emplace_back();
            transport = item.substr(0, first_ws);
            host      = hp->host;
            port      = hp->port;

            if (sep != std::string_view::npos) {
                params = take_param(rest.substr(sep), "branch", branch);
            }
        }

    }

    void parse_route_list(std::string_view value, std::vector<std::string>& out) {
        while (!value.empty()) {
            const auto item = next_list_item(value);
            if (!item.empty()) out.emplace_back(item);
        }
    }

    void parse_route(std::string_view, std::string_view value, SIPMessage& msg) {
        parse_route_list(value, msg.route);
    }

    void parse_record_route(std::string_view, std::string_view value, SIPMessage& msg) {
        parse_route_list(value, msg.record_route);
    }

    using Handler = void (*)(std::string_view name, std::string_view value, SIPMessage& msg);

    struct KnownHeader {
        std::string_view name;
        std::string_view compact;
        Handler handler;
        bool multi;
    };

    constexpr KnownHeader kKnownHeaders[] = {
        {"Call-ID",         "i", parse_call_id,         false},
        {"Content-Length",  "l", parse_content_length,  false},
        {"Content-Type",    "c", parse_content_type,    false},
        {"Max-Forwards",    "", parse_max_forwards,      false},
        {"Allow",           "", parse_allow,            true},
        {"From",        "f", parse_from,                false},
        {"To",          "t", parse_to,                  false},
        {"CSeq",            "", parse_cseq,             false},
        {"Supported",       "k", parse_supported,       true},
        {"Require",         "", parse_require,         true},
        {"Contact",     "m", parse_contact,             true},
        {"Via",         "v", parse_via,                 true},
        {"Route",           "", parse_route,            true},
        {"Record-Route",    "", parse_record_route,     true}
    };

    using SeenSet = std::bitset<std::size(kKnownHeaders)>;
    enum class DispatchResult : uint8_t { Known, Duplicate, Unknown };

    DispatchResult dispatch(std::string_view header_name, std::string_view value, SIPMessage& msg, SeenSet& seen) {
        for (std::size_t i = 0; i < std::size(kKnownHeaders); ++i) {
            const auto& k = kKnownHeaders[i];
            if (!iequals(header_name, k.name) && (k.compact.empty() || !iequals(header_name, k.compact))) continue;

            if (!k.multi && seen.test(i)) return DispatchResult::Duplicate;
            seen.set(i);
            k.handler(header_name, value, msg);
            return DispatchResult::Known;
        }

        return DispatchResult::Unknown;
    }

    void parse_rline(std::string_view sip_message, SIPMessage& msg) {
        const auto request_line_end = sip_message.find("\r\n");
        const auto start_line = sip_message.substr(0, request_line_end);
        const auto sp1 = start_line.find(' ');

        if (start_line.starts_with("SIP/2.0 ")) {
            msg.type = MessageType::RESPONSE;
            const auto sp2 = start_line.find(' ', sp1 + 1);
            msg.status_code = start_line.substr(sp1 + 1, sp2 - sp1 - 1);
            msg.reason = (sp2 == std::string_view::npos) ? std::string_view{} : start_line.substr(sp2 + 1);
        } else if (start_line.ends_with(" SIP/2.0")) {
            msg.type = MessageType::REQUEST;
            const auto sp2 = start_line.rfind(' ');
            msg.method = start_line.substr(0, sp1);
            msg.request_uri = start_line.substr(sp1 + 1, sp2 - sp1 - 1);
        }
    }

}

SIPParser::SIPParser(EventDispatcher* disp, SIPLogWriter* logger)
    : m_dispatcher(disp), m_logger(logger) {
    if (disp != nullptr) {
        m_dispatcher->register_listener(this);
    }
}

void SIPParser::on_event(Event& evt) {
    if (evt.type == EventType::RAW_MESSAGE_RECEIVED) {
        std::string* sip_raw = evt.raw_msg;
        auto parsed_message = parse_message(std::move(*sip_raw));

        Event e2;
        e2.type = EventType::MESSAGE_PARSED;
        //e2.parsed_msg = std::move(parsed_message);
        e2.ownership_claimed = false;
        m_dispatcher->dispatch(e2);
    } else if (evt.type == EventType::TIME_TO_REGISTER) {
        std::cout << "Event funktioniert" << std::endl;
    }
}

SIPMessage SIPParser::parse_message(std::string_view sip_message) {
    SIPMessage message{};

    const auto message_end = sip_message.find("\r\n\r\n");
    const auto request_line_end = sip_message.find("\r\n");
    message.is_complete = message_end != std::string_view::npos && request_line_end > 0;
    if (!message.is_complete) {
        //TODO: Logging
        return message;
    }

    parse_rline(sip_message, message);

    const auto headers_begin = request_line_end + 2;
    std::string_view sip_headers = message_end > headers_begin
        ? sip_message.substr(headers_begin, message_end - headers_begin)
        : std::string_view{};

    SeenSet seen{};

    while (!sip_headers.empty()) {
        const auto eol = sip_headers.find("\r\n");
        const auto line = sip_headers.substr(0, eol);
        sip_headers.remove_prefix(eol == std::string_view::npos ? sip_headers.size() : eol + 2);

        const auto colon = line.find(':');
        if (colon == std::string_view::npos) continue;

        const auto header_name = trim(line.substr(0, colon));
        const auto plain_value = trim(line.substr(colon + 1));

        switch (dispatch(header_name, plain_value, message, seen)) {
            case DispatchResult::Known:
                break;
            case DispatchResult::Duplicate:
                message.duplicate_header.push_back({std::string{header_name}, std::string{plain_value}});
                break;
            case DispatchResult::Unknown:
                message.unknown_header.push_back({std::string{header_name}, std::string{plain_value}});
                break;
        }
    }

    message.body = sip_message.substr(message_end + 4);

    return message;
}

