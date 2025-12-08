//
// Created by dkueh on 21.11.2025.
//

#include "SIPxApp.h"

#include <iostream>


SIPxApp::SIPxApp(const ModuleFlags flags, const std::string& filepath)
    : m_flags(flags) {
    if (m_flags.logwriter) {
        try {
            m_logger = std::make_unique<SIPLogWriter>(filepath);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    if (m_flags.parser) {
        m_parser = std::make_unique<SIPParser>(&m_dispatcher);
    }
    m_dispatcher.register_listener(this);
}

void SIPxApp::parse_raw_message(std::string&& raw_message) const {
    Event evt;
    evt.type = EventType::RAW_MESSAGE_RECEIVED;
    evt.raw_msg = &raw_message;
    m_dispatcher.dispatch(evt);
}

void SIPxApp::on_event(const Event& evt) {
    if (evt.type == EventType::MESSAGE_PARSED && evt.ownership_claimed == false) {
        m_msgs.push_back(std::unique_ptr<SIPMessage>(evt.parsed_msg));
        evt.ownership_claimed = true;
    }
}
