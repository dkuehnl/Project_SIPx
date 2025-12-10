//
// Created by dkueh on 08.12.2025.
//

#include "SIPNetwork.h"

#include <cstring>
#include <thread>
#include <ws2tcpip.h>

#include "platform/WindowsSocket.h"

/*** Constructor/Destructor ***/
SIPNetwork::SIPNetwork(EventDispatcher* disp, SIPLogWriter* log)
    : m_dispatcher(disp), m_logger(log) {
#ifdef _WIN32
    WSADATA wsa_data;
    const int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        SIPNetwork::log(std::format("WSAStartup() failed with error: {}", result));
    }
#endif

    m_dispatcher->register_listener(this);
}

SIPNetwork::~SIPNetwork() {
#ifdef _WIN32
    WSACleanup();
#endif
}


/*** Logging ***/
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

void SIPNetwork::on_event(const Event& evt) {
    std::lock_guard<std::mutex> lock(m_event_mutex);
    m_event_queue.push(evt);
}

void SIPNetwork::start() {
    m_running.store(true);
    m_network_thread = std::thread(&SIPNetwork::network_loop, this);
}

void SIPNetwork::stop() {
    m_running.store(false);
    if (m_network_thread.joinable()) {
        m_network_thread.join();
    }

    std::lock_guard<std::mutex>  lock(m_sockets_mutex);
    for (auto& p : m_sockets) {
        if (p.second.socket) p.second.socket->close();
    }

    m_sockets.clear();
}

/*** Main logic ***/
void SIPNetwork::network_loop() {
#ifdef _WIN32
    const int poll_timeout_ms = 50;
#else
    const int poll_timeout_ms = 100;
#endif

    while (m_running.load()) {
        process_internal_events();

#ifdef _WIN32
        std::vector<WSAPOLLFD> fds;
        {
            std::lock_guard<std::mutex> lock(m_sockets_mutex);
            fds.reserve(m_sockets.size());
            for (auto& kv : m_sockets) {
                WSAPOLLFD pfd;
                pfd.fd = static_cast<SOCKET>(kv.first);
                pfd.events = POLLRDNORM;
                pfd.revents = 0;
                fds.push_back(pfd);
            }
        }

        int ready = 0;
        if (!fds.empty()) {
            ready = WSAPoll(fds.data(), static_cast<ULONG>(fds.size()), poll_timeout_ms);
            if (ready == SOCKET_ERROR) {
                log(std::format("WSAPoll() failed with error: {}", WSAGetLastError()));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout_ms));
            continue;
        }

        if (ready > 0) {
            for (const auto& pfd : fds) {
                if (pfd.revents == 0) continue;
                if (pfd.revents & (POLLRDNORM | POLLIN)) {
                    intptr_t key = static_cast<intptr_t>(pfd.fd);
                    std::unique_lock<std::mutex> lock(m_sockets_mutex);
                    auto it = m_sockets.find(key);
                    if (it == m_sockets.end()) continue;
                    SocketEntry& entry = it->second;
                    lock.unlock();

                    process_recv_on_socket(entry);
                }
            }
        }

#else
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout_ms));
#endif
    }
}

void SIPNetwork::process_internal_events() {
    std::vector<Event> events;
    {
        std::lock_guard<std::mutex> lock(m_event_mutex);
        while (!m_event_queue.empty()) {
            events.push_back(std::move(m_event_queue.front()));
            m_event_queue.pop();
        }
    }

    for (const Event& evt: events) {
        handle_internal_events(evt);
    }
}

void SIPNetwork::handle_internal_events(const Event &evt) {
    switch (evt.type) {
        case EventType::CREATE_NETWORK_SOCKET: {
#ifdef _WIN32
            auto sock = std::make_unique<WindowsSocket>();
            if (evt.use_tcp) {
                if (!sock->create_tcp()) {
                    log(std::format("[SIPNetwork::handle_internal_events] Could not create tcp-socket {}", sock->get_last_error()));
                    return;
                }
                if (evt.source_port != 0) {
                    if (!sock->bind(evt.source_port)) {
                        log(std::format("[SIPNetwork::handle_internal_events] Could not bind tcp-socket {}", sock->get_last_error()));
                        return;
                    }
                }
                if (!sock->connect(evt.dest_ip.c_str(), evt.dest_port)) {
                    log(std::format("[SIPNetwork::handle_internal_events] Could not connect tcp-socket {}", sock->get_last_error()));
                    return;
                }
            } else {
                if (!sock->create_udp()) {
                    log(std::format("[SIPNetwork::handle_internal_events] Could not create udp-socket {}", sock->get_last_error()));
                    return;
                }
                if (evt.source_port != 0) {
                    if (!sock->bind(evt.source_port)) {
                        log(std::format("[SIPNetwork::handle_internal_events] Could not bind udp-socket {}", sock->get_last_error()));
                        return;
                    }
                }
            }

            intptr_t key = sock->get_handle();
            SocketEntry entry;
            entry.socket = std::move(sock);
            entry.is_tcp = evt.use_tcp;
            entry.dest_ip = evt.dest_ip;
            entry.dest_port = evt.dest_port;

            {
                std::lock_guard<std::mutex> lock(m_sockets_mutex);
                m_sockets.emplace(key, std::move(entry));
            }

            Event e2;
            e2.type = EventType::NETWORK_SOCKET_CREATED;
            m_dispatcher->dispatch(e2);
#endif
         break;
        }

        case EventType::SEND_MESSAGE: {
            const intptr_t key = evt.socket_handle;
            std::lock_guard<std::mutex> lock(m_sockets_mutex);
            auto it = m_sockets.find(key);
            if (it == m_sockets.end()) {
                log(std::format("[SIPNetwork::handle_internal_events] Could not find socket {}", evt.socket_handle));
                return;
            }

            const SocketEntry& entry = it->second;
            if (entry.is_tcp) {
                const int sent = entry.socket->send(evt.raw_msg->data(), evt.raw_msg->size());
                if (sent < 0) {
                    log(std::format("[SIPNetwork::handle_internal_events] TCP-Send failed: {}", entry.socket->get_last_error()));
                }
            } else {
                const int sent = entry.socket->send_to(evt.raw_msg->data(), evt.raw_msg->size(), entry.dest_ip.c_str(), entry.dest_port);
                if (sent < 0) {
                    log(std::format("[SIPNetwork::handle_internal_events] UDP-Send failed: {}", entry.socket->get_last_error()));
                }
            }

            break;
        }
        case EventType::SEND_MEDIA: {
            //tbd
            break;
        }
        default:
            break;
    }
}

void SIPNetwork::process_recv_on_socket(SocketEntry &entry) {
    if (entry.is_tcp) {
        char buffer[4096];
        while (true) {
            int rec = entry.socket->recv(buffer, sizeof(buffer));
            if (rec < 0) {
                int err = entry.socket->get_last_error();
                if (err == 10035) break; //WSAEWOULDBLOCK or EWOULDBLOCK
                log(std::format("[SIPNetwork::process_recv_on_socket] Could not receive data {}", err));
                break;
            }

            if (rec == 0) {
                intptr_t key = entry.socket->get_handle();
                entry.socket->close();
                std::lock_guard<std::mutex> lock(m_sockets_mutex);
                m_sockets.erase(static_cast<intptr_t>(key));
                break;
            }

            entry.recv_buffer.append(buffer, static_cast<size_t>(rec));
            while (true) {
                const size_t header_end = entry.recv_buffer.find("\r\n\r\n");
                if (header_end == std::string::npos) break;

                std::string header = entry.recv_buffer.substr(0, header_end + 4);
                size_t cl_pos = header.find("Content-Length:");
                size_t content_length = 0;
                if (cl_pos != std::string::npos) {
                    cl_pos += strlen("Content-Length:");

                    while (cl_pos < header.size() && (header[cl_pos] == ' ' || header[cl_pos] == '\t')) cl_pos++;
                    size_t line_end = header.find("\r\n", cl_pos);
                    if (line_end != std::string::npos && line_end > cl_pos) {
                        std::string cl_str = header.substr(cl_pos, line_end - cl_pos);
                        try {
                            content_length = std::stoul(cl_str);
                        } catch (...) {
                            content_length = 0;
                        }
                    }
                }

                const size_t total_needed = header_end + 4 + content_length;
                if (entry.recv_buffer.size() < total_needed) break;

                std::string complete_msg = entry.recv_buffer.substr(0, total_needed);
                entry.recv_buffer.erase(0, total_needed);

                dispatch_raw_message(std::move(complete_msg));
            }
        }
    } else {
        char buffer[65536];
        char src_ip[INET_ADDRSTRLEN] = {0};
        uint16_t src_port = 0;

        int rec = entry.socket->recv_from(buffer, sizeof(buffer), src_ip, &src_port);

        if (rec > 0) {
            std::string msg(buffer, static_cast<size_t>(rec));
            dispatch_raw_message(std::move(msg));
        } else if (rec < 0) {
            int err = entry.socket->get_last_error();
            log(std::format("[SIPNetwork::process_recv_on_socket] Could not receive data {}", err));
        }
    }
}

void SIPNetwork::dispatch_raw_message(std::string&& msg) {
    Event evt;
    evt.type = EventType::RAW_MESSAGE_RECEIVED;
    evt.raw_msg = new std::string(std::move(msg));
    m_dispatcher->dispatch(evt);
}