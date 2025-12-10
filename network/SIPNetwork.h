//
// Created by dkueh on 08.12.2025.
//

#ifndef PROJECT_SIPX_SIPNETWORK_H
#define PROJECT_SIPX_SIPNETWORK_H

#include <unordered_map>
#include <queue>
#include <mutex>
#include <thread>
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>
#include <string>
#include <atomic>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif

#include "../eventhandler/EventHandler.h"
#include "../eventhandler/EventDispatcher.h"
#include "../logwriter/SIPLogWriter.h"
#include "./platform/socket_base.h"

struct SocketEntry {
    std::unique_ptr<Socket> socket;
    bool is_tcp;
    std::string recv_buffer;
    std::string dest_ip;
    uint16_t dest_port{0};
};

class SIPNetwork : public EventHandler {
public:
    explicit SIPNetwork(EventDispatcher* disp = nullptr, SIPLogWriter* log = nullptr);
    ~SIPNetwork() override;

    void on_event(const Event& evt) override;
    void start();
    void stop();

private:
    EventDispatcher* m_dispatcher = nullptr;
    SIPLogWriter* m_logger = nullptr;

    std::atomic<bool> m_running{false};
    std::thread m_network_thread;

    std::mutex m_event_mutex;
    std::queue<Event> m_event_queue;

    std::mutex m_sockets_mutex;
    std::unordered_map<intptr_t, SocketEntry> m_sockets;


    void network_loop();
    void process_internal_events();
    void handle_internal_events(const Event& evt);
    void process_recv_on_socket(SocketEntry& entry);
    void dispatch_raw_message(std::string&& msg);

    void log(const std::string& context) const;
};


#endif //PROJECT_SIPX_SIPNETWORK_H