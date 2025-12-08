//
// Created by dkueh on 08.12.2025.
//

#include "SIPNetwork.h"

#include <format>

void SIPNetwork::log(const std::string& context) const {
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

bool SIPNetwork::send_tcp(const std::string& data) const {
    if (!m_tcp_socket) return false;

    int sent = m_tcp_socket->send(data.data(), data.size());
    if (sent < 0) {
        log(std::format("TCP send failed: {}", m_tcp_socket->get_last_error()));
        return false;
    }
    return true;
}

