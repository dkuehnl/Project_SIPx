//
// Created by dkueh on 05.11.2025.
//

#ifndef PROJECT_SIPX_SIPMESSAGE_H
#define PROJECT_SIPX_SIPMESSAGE_H

#include <string>
#include <vector>
#include <cstdint>

/// @brief Type of a parsed SIP message, derived from the start-line.
enum class MessageType : uint8_t {
    UNKNOWN,    ///< Start-line matches neither request nor response.
    REQUEST,
    RESPONSE
};

/**
 * @brief Decomposed name-addr / addr-spec of a From or To header.
 *
 * Without angle brackets, everything after the first ';' is treated as
 * header parameter (RFC 3261 §20).
 */
struct NameAddr {
    std::string scheme{};       ///< URI scheme, e.g. "sip", "sips", "tel".
    std::string user{};         ///< User part of the URI; complete number for tel-URIs.
    std::string host{};         ///< Host part, incl. brackets for IPv6 references.
    std::string port{};
    std::string uri_param{};    ///< Raw URI parameters, without leading ';'.
    std::string tag{};          ///< Value of the tag header parameter.
    std::string params{};       ///< Remaining header parameters (excluding tag), ';'-separated.

    bool operator==(const NameAddr&) const = default;
};

/**
 * @brief One via-parm of a Via header.
 *
 * Multiple Via lines and comma-separated values result in one entry each,
 * in order of appearance.
 */
struct Via {
    std::string transport{};    ///< Sent-protocol as received, e.g. "SIP/2.0/UDP".
    std::string host{};         ///< Sent-by host, incl. brackets for IPv6 references.
    std::string port{};         ///< Sent-by port; empty if not present.
    std::string branch{};       ///< Value of the branch parameter.
    std::string params{};       ///< Remaining parameters (excluding branch), ';'-separated.

    bool operator==(const Via&) const = default;
};

/// @brief Decomposed CSeq header.
struct CSeq {
    std::string number{};       ///< Sequence number as text.
    std::string method{};       ///< Method as received.

    bool operator==(const CSeq&) const = default;
};

/**
 * @brief One contact-param of a Contact header.
 *
 * Multiple Contact lines and comma-separated values result in one entry each,
 * in order of appearance.
 */
struct Contact {
    std::string scheme{};       ///< URI scheme, e.g. "sip", "sips", "tel".
    std::string user{};         ///< User part of the URI.
    std::string host{};         ///< Host part, incl. brackets for IPv6 references.
    std::string port{};         ///< URI port; empty if not present.
    std::string transport{};    ///< Value of the transport URI parameter.
    std::string uri_params{};   ///< Remaining URI parameters (excluding transport), ';'-separated.
    std::string expires{};      ///< Value of the expires header parameter.
    std::string params{};       ///< Remaining header parameters (excluding expires), ';'-separated.

    bool operator==(const Contact&) const = default; 
};

/// @brief Raw header as received, used for unknown, malformed and duplicate headers.
struct Header {
    std::string name{};         ///< Header name as received (long or compact form, original case).
    std::string value{};        ///< Raw value (whole header value or single list element), trimmed.
    bool operator==(const Header&) const = default;
};

/**
 * @brief Plain data struct of a parsed SIP message (request or response).
 *
 * Holds owned copies of all elements; the raw message is not referenced, so a
 * SIPMessage stays valid independently of the input buffer. All values are
 * stored as text exactly as received; numeric conversion is left to the module
 * that needs the value.
 *
 * An empty field means the header was missing, empty or malformed - the parser
 * does not distinguish these cases in the field itself. malformed_header and
 * duplicate_header show what could not be assigned.
 *
 * @note If is_complete is false, all other fields are left default.
 */
struct SIPMessage {
    MessageType type{MessageType::UNKNOWN};     ///< Detected from the start-line.
    bool is_complete{false};                    ///< false if no header terminator (CRLFCRLF) was found.

    std::string method{};                       ///< Request only.
    std::string request_uri{};                  ///< Request only. Raw Request-URI.
    std::string status_code{};                  ///< Response only. Status code as text.
    std::string reason{};                       ///< Response only. Reason phrase; empty if not present.

    NameAddr                    from{};                     ///< From / f
    NameAddr                    to{};                       ///< To / t
    std::vector<Via>            via{};                      ///< Via / v - all via-parms in order of appearance.
    std::vector<std::string>    route{};                    ///< Route - raw entries in order of appearance.
    std::vector<std::string>    record_route{};             ///< Record-Route - raw entries in order of appearance.
    CSeq                        cseq{};                     ///< CSeq
    std::vector<Contact>        contacts{};                 ///< Contact / m - all contact-params in order of appearance.
    std::string                 call_id{};                  ///< Call-ID / i
    std::string                 content_length{};           ///< Content-Length / l. Not used for body framing.
    std::string                 content_type{};             ///< Content-Type / c
    std::string                 max_forwards{};             ///< Max-Forwards
    std::vector<std::string>    allow{};                    ///< Allow - method tokens across all lines.
    std::vector<std::string>    supported{};                ///< Supported / k - option tags across all lines.
    std::vector<std::string>    require{};                  ///< Require - option tags across all lines.

    std::vector<Header>         unknown_header{};           ///< Headers without dedicated field, in order of appearance. Repetitions allowed.
    std::vector<Header>         malformed_header{};         ///< Known headers or list elements that could not be decomposed.
    std::vector<Header>         duplicate_header{};         ///< Repeated single-value headers, unparsed. First occurrence occupies the field.
    std::string                 malformed_start_line{};     ///< for non RFC-conform start-line
    std::string                 body{};                     ///< Everything after the header terminator, raw.
};

#endif //PROJECT_SIPX_SIPMESSAGE_H