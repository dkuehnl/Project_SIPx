//
// Created by dkueh on 08.12.2025.
//

#ifndef PROJECT_SIPX_SIPNETWORK_H
#define PROJECT_SIPX_SIPNETWORK_H

#include "../eventhandler/EventHandler.h"
#include "../eventhandler/EventDispatcher.h"
#include "../logwriter/SIPLogWriter.h"
#include "./platform/socket_base.h"

class SIPNetwork : public EventHandler {
public:
    explicit SIPNetwork(EventDispatcher* disp = nullptr, SIPLogWriter* log = nullptr)
        : m_dispatcher(disp), m_logger(log) {}

    bool establish_sip_tcp(const std::string& dest_ip, uint16_t dest_port = 5060, uint16_t source_port = 0);
    bool establish_sip_udp(const std::string& dest_ip, uint16_t dest_port = 5060, uint16_t source_port = 0);

    bool send_tcp(const std::string& data) const;
    bool send_udp(const std::string& data);

    bool receive_tcp(std::string& out_message);
    bool receive_udp(std::string& out_message);

    void on_event(const Event& evt) override;

private:
    EventDispatcher* m_dispatcher = nullptr;
    SIPLogWriter* m_logger = nullptr;

    std::unique_ptr<Socket> m_tcp_socket;
    std::unique_ptr<Socket> m_udp_socket;

    std::string m_tcp_recv_buffer;

    void log(const std::string& context) const;
};


#endif //PROJECT_SIPX_SIPNETWORK_H