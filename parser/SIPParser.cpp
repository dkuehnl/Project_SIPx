//
// Created by dkueh on 05.11.2025.
//

#include "SIPParser.h"

#include <iostream>
#include <algorithm>
#include <optional>
#include <bitset>

#include "../eventhandler/EventDispatcher.h"
#include "../logwriter/SIPLogWriter.h"

namespace {
    /// @brief Views on the parts of a SIP/SIPS/TEL URI; all point into the input.
    struct UriParts {
        std::string_view scheme{};
        std::string_view user{};
        std::string_view host{};
        std::string_view port{};
        std::string_view params{};
        std::string_view headers{};
    };

    /// @brief addr-spec and the raw header parameters following it.
    struct NameAddrParts {
        std::string_view addr{};
        std::string_view params{};
    };

    /// @brief Host and optional port of a hostport / sent-by.
    struct HostPort {
        std::string_view host{};
        std::string_view port{};
    };

    constexpr std::string_view SipVersion = "SIP/2.0";
    /// @brief true for SP and HTAB.
    constexpr bool is_wsp(char c) noexcept { return c == ' ' || c == '\t'; }

    /// @brief Strips leading and trailing SP/HTAB.
    constexpr std::string_view trim(std::string_view s) noexcept {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
        return s;
    }

    constexpr char ascii_lower(char c) noexcept {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
    }

    /// @brief ASCII case-insensitive comparison.
    constexpr bool iequals(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (ascii_lower(a[i]) != ascii_lower(b[i])) return false;
        }

        return true;
    }

    /// @brief Position of the closing '"' of a quoted-string starting at s[0]; honours '\\' escapes.
    std::size_t find_closing_quote(std::string_view s) noexcept {
        for (std::size_t i = 1; i < s.size(); ++i) {
            if (s[i] == '\\') { ++i; continue; }
            if (s[i] == '"') return i;
        }

        return std::string_view::npos;
    }

    /**
     * @brief Splits host [SWS ':' SWS port]; IPv6 references keep their brackets.
     * @return nullopt for an empty host, an unclosed '[' or trailing text after the host.
     */
    std::optional<HostPort> split_hostport(std::string_view value) {
        std::size_t host_end = 0;
        if (!value.empty() && value.front() == '[') {
            const auto rb = value.find(']');
            if (rb == std::string_view::npos) return std::nullopt;
            host_end = rb + 1;
        } else {
            host_end = std::min(value.find_first_of(" \t:"), value.size());
        }

        HostPort hp{value.substr(0, host_end), {}};
        if (hp.host.empty()) return std::nullopt;

        const auto rest = trim(value.substr(host_end));
        if (rest.empty()) return hp;
        if (rest.front() != ':') return std::nullopt;

        hp.port = trim(rest.substr(1));
        return hp;
    }

    /**
     * @brief Splits a URI into scheme, user, host, port, parameters and headers.
     *
     * URI headers ('?' ...) are cut off first, then parameters (';' ...). For tel-URIs
     * the complete number ends up in user.
     * @return nullopt for a missing or empty scheme or an invalid hostport.
     */
    std::optional<UriParts> split_uri(std::string_view uri) {
        UriParts p;

        const auto colon = uri.find(':');
        if (colon == std::string_view::npos || colon == 0) return std::nullopt;
        p.scheme = uri.substr(0, colon);
        uri.remove_prefix(colon + 1);

        if (const auto q = uri.find('?'); q != std::string_view::npos) {
            p.headers = uri.substr(q + 1);
            uri = uri.substr(0, q);
        }

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

        const auto host_port = split_hostport(uri);
        if (!host_port) return std::nullopt;

        p.host = host_port->host;
        p.port = host_port->port;

        return p;
    }

    /**
     * @brief Separates name-addr / addr-spec from the header parameters.
     * @return nullopt for an unclosed '"' or '<', or a quoted display-name without '<'.
     */
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

        if (search_from != 0) return std::nullopt;

        const auto semi = value.find(';');
        return NameAddrParts{trim(value.substr(0, semi)),
            semi == std::string_view::npos
            ? std::string_view{}
            : value.substr(semi) };
    }

    /**
     * @brief Extracts the first parameter named key (case-insensitive) into value.
     * @return All remaining parameters incl. repeated keys, ';'-separated and trimmed.
     */
    std::string take_param(std::string_view params, std::string_view key, std::string& value) {
        std::string rest;
        bool found{false};

        params = trim(params);
        if (!params.empty() && params.front() == ';') params.remove_prefix(1);

        while (!params.empty()) {
            const auto semi = params.find(';');
            const auto param = trim(params.substr(0, semi));
            params.remove_prefix(semi == std::string_view::npos ? params.size() : semi + 1);

            if (param.empty()) continue;

            const auto eq = param.find('=');
            if (!found && eq != std::string_view::npos && iequals(trim(param.substr(0, eq)), key)) {
                value = trim(param.substr(eq + 1));
                found = true;
                continue;
            }

            if (!rest.empty()) rest += ';';
            rest += param;
         }
        return rest;
    }

    /// @brief Appends URI headers as "?headers" to the URI parameters, if present.
    std::string join_uri_params(std::string params, std::string_view headers) {
        if (!headers.empty()) {
            params += '?';
            params += headers;
        }

        return params;
    }

    /// @brief Returns the next comma-separated element; commas inside quotes and '<...>' are ignored.
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

    /// @brief Stores a header or list element that could not be decomposed.
    void add_malformed(SIPMessage& msg, std::string_view name, std::string_view value) {
        msg.malformed_header.push_back({std::string{name}, std::string{value}});
    }

    /// @brief Decomposes a From/To value into out; false if it is malformed.
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
        out.port    = uri->port;
        out.uri_params = join_uri_params(std::string{uri->params}, uri->headers);
        out.params  = take_param(na->params, "tag", out.tag);

        return true;
    }

    /// @brief Returns the next line without CRLF and advances rest.
    std::string_view next_line(std::string_view& rest) noexcept {
        const auto eol = rest.find("\r\n");
        const auto line = rest.substr(0, eol);
        rest.remove_prefix(eol == std::string_view::npos ? rest.size() : eol + 2);

        return line;
    }

    /**
     * @brief Returns the next logical header line incl. its continuation lines.
     *
     * Without folding the view points into rest, otherwise into unfold_buf (each LWS run
     * replaced by a single SP). The view is only valid until the next call.
     */
    std::string_view next_header_line(std::string_view& rest, std::string& unfold_buf) {
        const auto first = next_line(rest);
        if (rest.empty() || !is_wsp(rest.front())) return first;

        unfold_buf.assign(first);
        while (!rest.empty() && is_wsp(rest.front())) {
            const auto cont = trim(next_line(rest));
            if (cont.empty()) continue;

            while (!unfold_buf.empty() && is_wsp(unfold_buf.back())) unfold_buf.pop_back();
            unfold_buf += ' ';
            unfold_buf += cont;
        }

        return unfold_buf;
    }

    /// @brief Splits a comma-separated token list; empty elements are skipped.
    void parse_option_tags(std::string_view value, std::vector<std::string>& out) {
        while (!value.empty()) {
            const auto comma = value.find(',');
            const auto tag = trim(value.substr(0, comma));
            value.remove_prefix(comma == std::string_view::npos ? value.size() : comma + 1);

            if (!tag.empty()) out.emplace_back(tag);
        }
    }

    void parse_from(std::string_view name, std::string_view value, SIPMessage& msg) {
        if (!parse_name_addr(value, msg.from)) add_malformed(msg, name, value);
    }

    void parse_to(std::string_view name, std::string_view value, SIPMessage& msg) {
        if (!parse_name_addr(value, msg.to)) add_malformed(msg, name, value);
    }

    void parse_cseq(std::string_view name, std::string_view value, SIPMessage& msg) {
        const auto sep = value.find_first_of(" \t");

        if (sep == std::string_view::npos) {
            add_malformed(msg, name, value);
            //TODO Logging
            return;
        }

        msg.cseq.number = trim(value.substr(0, sep));
        msg.cseq.method = trim(value.substr(sep + 1));
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
            c.uri_params = join_uri_params(take_param(uri->params, "transport", c.transport), uri->headers);
            c.params    = take_param(na->params, "expires", c.expires);
        }
    }

    /**
     * @brief Reads sent-protocol (name SLASH version SLASH transport) into out without whitespace.
     * @return Remainder after the separating LWS (sent-by and parameters), or nullopt if malformed.
     */
    std::optional<std::string_view> take_sent_protocol(std::string_view item, std::string& out) {
        out.clear();

        for (int i = 0; i < 3; ++i) {
            item = trim(item);
            const auto token = item.substr(0, item.find_first_of(" \t/"));
            if (token.empty()) return std::nullopt;

            out += token;
            item.remove_prefix(token.size());
            if (i == 2) break;

            item = trim(item);
            if (item.empty() || item.front() != '/') return std::nullopt;
            out += '/';
            item.remove_prefix(1);
        }

        if (item.empty() || !is_wsp(item.front())) return std::nullopt;
        return trim(item);
    }

    void parse_via(std::string_view name, std::string_view value, SIPMessage& msg) {
        std::string sent_protocol;

        while (!value.empty()) {
            const auto item = next_list_item(value);
            if (item.empty()) continue;

            const auto rest = take_sent_protocol(item, sent_protocol);
            if (!rest) {
                add_malformed(msg, name, item);
                //TODO: Logging
                continue;
            }

            const auto sep = rest->find(';');
            const auto hp = split_hostport(trim(rest->substr(0, sep)));
            if (!hp) {
                add_malformed(msg, name, item);
                //TODO: Logging
                continue;
            }

            auto& [transport, host, port, branch, params] = msg.via.emplace_back();
            transport = std::move(sent_protocol);
            host      = hp->host;
            port      = hp->port;

            if (sep != std::string_view::npos) {
                params = take_param(rest->substr(sep), "branch", branch);
            }
        }
    }

    /// @brief Stores every non-empty list element raw.
    void parse_route_list(std::string_view value, std::vector<std::string>& out) {
        while (!value.empty()) {
            const auto item = next_list_item(value);
            if (!item.empty()) out.emplace_back(item);
        }
    }

    /// @brief Handler signature: header name as received, trimmed value, target message.
    using Handler = void (*)(std::string_view name, std::string_view value, SIPMessage& msg);

    /// @brief Entry of the known-header table; multi marks list headers allowed to repeat.
    struct KnownHeader {
        std::string_view name;
        std::string_view compact;
        Handler handler;
        bool multi;
    };

    /// @brief Handler for plain single-value text headers.
    template <std::string SIPMessage::* Field>
    void parse_text(std::string_view, std::string_view value, SIPMessage& msg) {
        msg.*Field = value;
    }

    /// @brief Handler for comma-separated token lists (Allow, Supported, Require).
    template <std::vector<std::string> SIPMessage::* Field>
    void parse_tags(std::string_view, std::string_view value, SIPMessage& msg) {
        parse_option_tags(value, msg.*Field);
    }

    /// @brief Handler for Route / Record-Route, entries stored raw.
    template <std::vector<std::string> SIPMessage::*Field>
    void parse_routes(std::string_view, std::string_view value, SIPMessage& msg) {
        parse_route_list(value, msg.*Field);
    }

    /// @brief All headers with a dedicated field in SIPMessage.
    constexpr KnownHeader kKnownHeaders[] = {
        {"Call-ID",         "i", parse_text<&SIPMessage::call_id>,         false},
        {"Content-Length",  "l", parse_text<&SIPMessage::content_length>,  false},
        {"Content-Type",    "c", parse_text<&SIPMessage::content_type>,    false},
        {"Max-Forwards",    "",  parse_text<&SIPMessage::max_forwards>,    false},
        {"Allow",           "",  parse_tags<&SIPMessage::allow>,           true},
        {"From",            "f", parse_from,                               false},
        {"To",              "t", parse_to,                                 false},
        {"CSeq",            "",  parse_cseq,                               false},
        {"Supported",       "k", parse_tags<&SIPMessage::supported>,       true},
        {"Require",         "",  parse_tags<&SIPMessage::require>,         true},
        {"Contact",         "m", parse_contact,                            true},
        {"Via",             "v", parse_via,                                true},
        {"Route",           "",  parse_routes<&SIPMessage::route>,         true},
        {"Record-Route",    "",  parse_routes<&SIPMessage::record_route>,  true}
    };

    /// @brief One bit per known header, used for duplicate detection of single-value headers.
    using SeenSet = std::bitset<std::size(kKnownHeaders)>;
    enum class DispatchResult : uint8_t { Known, Duplicate, Unknown };

    /// @brief Looks up header_name in kKnownHeaders and calls its handler unless it is a duplicate.
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

    /// @brief Parses "SIP/2.0" SP code [SP reason]; leaves msg untouched on failure.
    bool parse_status_line(std::string_view line, SIPMessage& msg) {
        if (!line.starts_with(SipVersion)
            || line.size() <= SipVersion.size()
            || line[SipVersion.size()] != ' ') return false;

        line.remove_prefix(SipVersion.size() + 1);
        const auto sp = line.find(' ');
        const auto code = line.substr(0, sp);
        if (code.empty()) return false;

        msg.type = MessageType::RESPONSE;
        msg.status_code = code;
        if (sp != std::string_view::npos) msg.reason = line.substr(sp + 1);
        return true;
    }

    /// @brief Parses Method SP Request-URI SP "SIP/2.0"; leaves msg untouched on failure.
    bool parse_request_line(std::string_view line, SIPMessage& msg) {
        const auto sp1 = line.find(' ');
        const auto sp2 = line.rfind(' ');
        if (sp1 == std::string_view::npos || sp1 == sp2) return false;

        const auto method = line.substr(0, sp1);
        const auto uri = line.substr(sp1 + 1, sp2 - sp1 - 1);
        const auto version = line.substr(sp2 + 1);

        if (method.empty() || uri.empty()
            || uri.find(' ') != std::string_view::npos
            || version != SipVersion) return false;

        msg.type = MessageType::REQUEST;
        msg.method = method;
        msg.request_uri = uri;

        return true;
    }

    /// @brief Detects request or response; stores the raw line as malformed_start_line otherwise.
    void parse_start_line(std::string_view line, SIPMessage& msg) {
        if (parse_status_line(line, msg) || parse_request_line(line, msg)) return;
        msg.malformed_start_line = line;

        //TODO: Logging
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
        m_incomplete_pkt_count++;
        m_total_pkt_count++;
        return message;
    }

    parse_start_line(sip_message.substr(0, request_line_end), message);

    const auto headers_begin = request_line_end + 2;
    std::string_view sip_headers = message_end > headers_begin
        ? sip_message.substr(headers_begin, message_end - headers_begin)
        : std::string_view{};

    SeenSet seen{};
    std::string unfold_buf{};

    while (!sip_headers.empty()) {
        const auto line = next_header_line(sip_headers, unfold_buf);
        if ( line.empty()) continue;

        const auto colon = line.find(':');
        const auto header_name = colon == std::string_view::npos
            ? std::string_view{}
            : trim(line.substr(0, colon));

        if (is_wsp(line.front()) || header_name.empty()) {
            add_malformed(message, {}, trim(line));
            //TODO: Logging
            continue;
        }

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

    m_total_pkt_count++;
    return message;
}

