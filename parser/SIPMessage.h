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
    UNKNOWN,    ///< Start-line is neither a Request-Line nor a Status-Line; see SIPMessage::malformed_start_line.
    REQUEST,    ///< Request-Line: Method SP Request-URI SP "SIP/2.0".
    RESPONSE    ///< Status-Line: "SIP/2.0" SP Status-Code [SP Reason-Phrase].
};

/**
 * @brief Decomposed name-addr / addr-spec of a From or To header.
 *
 * Without angle brackets, everything after the first ';' is treated as
 * header parameter (RFC 3261 §20). A quoted display-name requires angle
 * brackets, otherwise the header is malformed. The display-name itself is
 * not stored.
 *
 * Header parameters: the first occurrence of tag fills the field, repeated
 * ones stay in params.
 */
struct NameAddr {
    std::string scheme{};       ///< URI scheme, e.g. "sip", "sips", "tel".
    std::string user{};         ///< User part of the URI; complete number for tel-URIs.
    std::string host{};         ///< Host part, incl. brackets for IPv6 references.
    std::string port{};         ///< URI port; empty if not present.
    std::string uri_params{};   ///< Raw URI parameters, without leading ';'. URI headers appended as "?hname=hvalue&...".
    std::string tag{};          ///< Value of the first tag header parameter.
    std::string params{};       ///< Remaining header parameters incl. repeated tags, ';'-separated.

    bool operator==(const NameAddr&) const = default;
};

/**
 * @brief One via-parm of a Via header.
 *
 * Multiple Via lines and comma-separated values result in one entry each,
 * in order of appearance. Whitespace around '/' and ':' is accepted
 * (SLASH / COLON = SWS, RFC 3261 §25.1). An entry without sent-by host or
 * with trailing text after it is malformed.
 */
struct Via {
    std::string transport{};    ///< Sent-protocol without surrounding whitespace, e.g. "SIP/2.0/UDP".
    std::string host{};         ///< Sent-by host, incl. brackets for IPv6 references.
    std::string port{};         ///< Sent-by port; empty if not present.
    std::string branch{};       ///< Value of the first branch parameter.
    std::string params{};       ///< Remaining parameters incl. repeated branch, ';'-separated.

    bool operator==(const Via&) const = default;
};

/// @brief Decomposed CSeq header; number and method are separated by SP or HTAB.
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
    std::string transport{};    ///< Value of the first transport URI parameter.
    std::string uri_params{};   ///< Remaining URI parameters incl. repeated transport, ';'-separated. URI headers appended as "?hname=hvalue&...".
    std::string expires{};      ///< Value of the first expires header parameter.
    std::string params{};       ///< Remaining header parameters incl. repeated expires, ';'-separated.

    bool operator==(const Contact&) const = default;
};

/**
 * @brief Raw header as received, used for unknown, malformed and duplicate headers.
 *
 * Folded header lines are stored unfolded (each LWS run replaced by a single SP).
 * Lines that are no valid header - no ':', empty name or continuation line
 * without preceding header - are stored with an empty name and the whole line
 * as value.
 */
struct Header {
    std::string name{};         ///< Header name as received (long or compact form, original case); empty for invalid lines.
    std::string value{};        ///< Value (whole header value or single list element), trimmed and unfolded.

    bool operator==(const Header&) const = default;
};

/**
 * @brief Plain data struct of a parsed SIP message (request or response).
 *
 * Holds owned copies of all elements; the raw message is not referenced, so a
 * SIPMessage stays valid independently of the input buffer. All values are
 * stored as text as received - only folded lines are unfolded and optional
 * whitespace around separators is dropped. Numeric conversion is left to the
 * module that needs the value.
 *
 * An empty field means the header was missing, empty or malformed - the parser
 * does not distinguish these cases in the field itself. malformed_header and
 * duplicate_header show what could not be assigned.
 *
 * @note If is_complete is false, all other fields are left default.
 * @note Header names in unknown_header are stored as received; compact forms
 *       without dedicated field (e.g. "s" for Subject) are not mapped to their
 *       long form.
 */
struct SIPMessage {
    MessageType type{MessageType::UNKNOWN};     ///< Detected from the start-line.
    bool is_complete{false};                    ///< false if no header terminator (CRLFCRLF) was found or the input starts with CRLF.

    std::string method{};                       ///< Request only. Method as received.
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
    std::vector<Header>         malformed_header{};         ///< Known headers or list elements that could not be decomposed; invalid lines with empty name.
    std::vector<Header>         duplicate_header{};         ///< Repeated single-value headers, unparsed. First occurrence occupies the field.
    std::string                 malformed_start_line{};     ///< Raw start-line if type is UNKNOWN; empty otherwise.
    std::string                 body{};                     ///< Everything after the header terminator, raw.
};

#endif //PROJECT_SIPX_SIPMESSAGE_H