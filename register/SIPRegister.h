//
// Created by dkueh on 16.12.2025.
//

#ifndef PROJECT_SIPX_SIPREGISTER_H
#define PROJECT_SIPX_SIPREGISTER_H

#include "../eventhandler/EventHandler.h"
#include "../eventhandler/EventDispatcher.h"
#include "../logwriter/SIPLogWriter.h"
#include "../parser/SIPRequest.h"
#include "../parser/SIPResponse.h"


class SIPRegister final : public EventHandler {
public:
    explicit SIPRegister(EventDispatcher* disp = nullptr, SIPLogWriter* log = nullptr)
        : m_dispatcher(disp), m_logger(log) { m_dispatcher->register_listener(this); }

    void on_event(Event& evt) override;
    SIPRequest build_request();
    std::unique_ptr<SIPResponse> build_response(uint16_t response_code);

private:
    EventDispatcher* m_dispatcher;
    SIPLogWriter* m_logger;
    std::unordered_map<std::string, std::vector<std::unique_ptr<SIPMessage>>> m_register_messages;

    int16_t extract_expire(const SIPResponse& resp);
    std::string extract_auth_header(const SIPResponse& resp);
    std::string build_auth_header(const std::string& resp_auth_header);
    void store_register_message(std::unique_ptr<SIPMessage> msg);

    void log(const std::string& message) const;
};


#endif //PROJECT_SIPX_SIPREGISTER_H