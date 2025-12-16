//
// Created by dkueh on 16.12.2025.
//

#include "SIPRegister.h"
#include "../utils/sip_utils.h"

/*** Logging ***/

void SIPRegister::log(const std::string& context) const {
    if (m_dispatcher) {
        Event evt;
        evt.type = EventType::LOG_MESSAGE;
        evt.log_message = context;
        m_dispatcher->dispatch(evt);
    }
    if (m_logger) {
        m_logger->write_log(context);
    }
}

void SIPRegister::store_register_message(std::unique_ptr<SIPMessage> msg) {
    if (!msg) return;

    const std::string key{ msg->call_id() };
    m_register_messages[key].push_back(std::move(msg));
}

void SIPRegister::on_event(Event& evt) {
    switch (evt.type) {
        case EventType::TIME_TO_REGISTER: {
            Event evt2;
            //socket für Connection bereits verfügbar?
            if (evt.connection->socket_active) {
                auto reg_msg = build_request();

                evt2.type = EventType::SEND_MESSAGE;
                evt2.parsed_msg = &reg_msg;
                m_dispatcher->dispatch(evt2);
                break;
            }

            evt2.type = EventType::CREATE_NETWORK_SOCKET;
            evt2.connection = evt.connection;
            m_dispatcher->dispatch(evt2);
            break;
        }
        case EventType::MESSAGE_PARSED: {
            if (evt.parsed_msg->get_cseq_typ() == "REGISTER") {
                auto& msg = evt.parsed_msg;
                if (!msg) {
                    log(std::format("[SIPRegister] on_event(): no parsed message found."));
                    break;
                }

                //prüfe ob Request oder Response
                if (!utils::sip::is_response(msg->request_line())) {
                    //wenn Requste => ungültig, 400 Bad Request schicken
                    log(std::format("[SIPRegister] on_event(): Register-request received, but I'm not a SIP-proxy"));

                    auto response = build_response(400);
                    std::string key{msg->call_id()};
                    auto& vec = m_register_messages[key];
                    vec.push_back(std::move(evt.parsed_msg));
                    vec.push_back(std::move(response));

                    Event evt2;
                    evt2.type = EventType::SEND_MESSAGE;
                    evt2.raw_sip_message = vec.back().get();
                    evt2.connection = evt.connection;
                    m_dispatcher->dispatch(evt2);

                    break;
                }
                //wenn Response => Status-Line holen und auf 401/407/200 testen

                //wenn Error-Response => tbd
                //Wenn 401/407 => Auth-Header extrahieren und mit PW Auth-Response berechnen
                //Wenn 200OK => Contact extrahieren, zu URI matchen und expire speichern
            }
            break;
        }
        case EventType::NETWORK_SOCKET_CREATED: {
            if (!evt.connection->socket_active) {
                log(std::format("[SIPRegister] on_event(): Inconsistent state. "
                                "NETWORK_SOCKET_CREATED-event detected, but connection-bool set to false."));
                break;
            }

            Event evt2;
            auto reg_msg = build_request();

            evt2.type = EventType::SEND_MESSAGE;
            evt2.parsed_msg = &reg_msg;
            m_dispatcher->dispatch(evt2);
            break;
        }
        case EventType::NETWORK_FAILED:
            break;
        default:
            break;
    }
}

