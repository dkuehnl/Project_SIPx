//
// Created by dkueh on 05.11.2025.
//

#ifndef PROJECT_SIPX_SIPPARSER_H
#define PROJECT_SIPX_SIPPARSER_H

#include "SIPMessage.h"
#include "../eventhandler/EventHandler.h"


class EventDispatcher;
class SIPLogWriter;

/**
 * @brief Parses raw SIP messages into plain SIPMessage structs.
 *
 * Handles requests and responses alike; the detected type is stored in
 * SIPMessage::type. The parser only extracts text: no numeric conversion,
 * no semantic validation, no modification or repair of the input.
 *
 * The start-line is checked structurally only: a Request-Line needs exactly three
 * non-empty tokens separated by single SP and ending in "SIP/2.0", a Status-Line
 * starts with "SIP/2.0" SP and a non-empty status code. Anything else sets
 * SIPMessage::type to UNKNOWN and stores the raw line in
 * SIPMessage::malformed_start_line; headers and body are parsed regardless.
 *
 * Folded header lines (continuation lines starting with SP/HTAB) are unfolded
 * before parsing, each LWS run replaced by a single SP (RFC 3261 §7.3.1, §25.1).
 *
 * Known headers are matched case-insensitively in long and compact form.
 * Every header line ends up in exactly one place:
 *  - known header                         -> its dedicated field
 *  - known header that cannot be split    -> SIPMessage::malformed_header (field stays empty)
 *  - repeated single-value header         -> SIPMessage::duplicate_header, unparsed; the first
 *                                            occurrence occupies the field, even if malformed
 *  - malformed element of a list header   -> SIPMessage::malformed_header; valid elements
 *                                            stay in the field
 *  - unknown header                       -> SIPMessage::unknown_header (repetitions allowed)
 *  - line that is no valid header         -> SIPMessage::malformed_header with empty name and
 *    (no ':', empty name, continuation       the whole line as value
 *    line without preceding header)
 * Missing or empty headers simply leave the field empty; no evaluation takes place.
 * Repeated parameters (tag, branch, transport, expires): the first occurrence fills
 * the field, further ones stay in the remaining parameters.
 *
 * @note Framing is the caller's responsibility: parse_message() expects exactly one
 *       complete message with CRLF line endings.
 * @note Dispatcher and logger are optional and not owned; the caller must keep them
 *       alive for the lifetime of the parser.
 * @note Not thread-safe: parse_message() updates the packet counters. Use one
 *       instance per thread.
 * @todo Leading CRLFs before the start-line currently result in is_complete == false.
 * @todo on_event() does not forward the parsed message yet (Event::parsed_msg).
 */
class SIPParser : public EventHandler {
public:
    /**
     * @brief Constructs the parser and registers it as listener if a dispatcher is given.
     * @param disp Optional EventDispatcher; referenced, not owned.
     * @param logger Optional SIPLogWriter; referenced, not owned. Logging is disabled if nullptr.
     */
    explicit SIPParser(EventDispatcher* disp = nullptr, SIPLogWriter* logger = nullptr);

    /**
     * @brief Parses a single raw SIP message.
     *
     * The input is only read; all extracted values are copied into the returned
     * struct, so the result stays valid after the input buffer is released.
     *
     * @param sip_message Complete raw SIP message incl. header terminator (CRLFCRLF).
     * @return Parsed message. SIPMessage::is_complete is false if no header
     *         terminator was found or the input starts with CRLF; all other
     *         fields are left empty in that case.
     */
    SIPMessage parse_message(std::string_view sip_message);

    /// @brief Number of parse_message() calls on this instance, incl. incomplete messages.
    [[nodiscard]] unsigned int get_total_pkt_count() const { return m_total_pkt_count; }

    /// @brief Number of parse_message() calls that returned SIPMessage::is_complete == false.
    [[nodiscard]] unsigned int get_incomplete_pkt_count() const { return m_incomplete_pkt_count; }

    /**
     * @brief Handles RAW_MESSAGE_RECEIVED by parsing the message and dispatching MESSAGE_PARSED.
     * @param evt Incoming event; evt.raw_msg must point to a valid string for RAW_MESSAGE_RECEIVED.
     */
    void on_event(Event& evt) override;

private:
    EventDispatcher* m_dispatcher = nullptr;    ///< Optional, not owned.
    SIPLogWriter* m_logger = nullptr;           ///< Optional, not owned.
    unsigned int m_total_pkt_count{0};          ///< Incremented per parse_message() call.
    unsigned int m_incomplete_pkt_count{0};     ///< Incremented per incomplete message.

};


#endif //PROJECT_SIPX_SIPPARSER_H