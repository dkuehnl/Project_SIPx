//
// Created by dkueh on 05.11.2025.
//

#ifndef PROJECT_SIPX_SIPPARSER_H
#define PROJECT_SIPX_SIPPARSER_H

#include "SIPMessage.h"
#include "../eventhandler/EventHandler.h"
#include "../eventhandler/EventDispatcher.h"
#include "../logwriter/SIPLogWriter.h"

/**
 * @brief Parses raw SIP messages into plain SIPMessage structs.
 *
 * Handles requests and responses alike; the detected type is stored in
 * SIPMessage::type. The parser only extracts text: no numeric conversion,
 * no semantic validation, no modification or repair of the input.
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
 * Missing or empty headers simply leave the field empty; no evaluation takes place.
 *
 * @note Framing is the caller's responsibility: parse_message() expects exactly one
 *       complete message.
 * @note Dispatcher and logger are optional and not owned; the caller must keep them
 *       alive for the lifetime of the parser.
 * @todo Line folding (continuation lines starting with SP/HTAB) is not supported yet.
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
     *         terminator was found; all other fields are left empty in that case.
     */
    SIPMessage parse_message(std::string_view sip_message);

    /// @brief Number of messages parsed by this instance.
    [[nodiscard]] unsigned int get_paket_count() const { return m_paket_count; }

    /**
     * @brief Handles RAW_MESSAGE_RECEIVED by parsing the message and dispatching MESSAGE_PARSED.
     * @param evt Incoming event; evt.raw_msg must point to a valid string for RAW_MESSAGE_RECEIVED.
     */
    void on_event(Event& evt) override;

private:
    EventDispatcher* m_dispatcher = nullptr;    ///< Optional, not owned.
    SIPLogWriter* m_logger = nullptr;           ///< Optional, not owned.
    unsigned int m_paket_count = 0;             ///< Incremented per parse_message() call.

};


#endif //PROJECT_SIPX_SIPPARSER_H