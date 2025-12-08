//
// Created by dkueh on 08.12.2025.
//

#ifndef PROJECT_SIPX_SOCKET_WINDOWS_H
#define PROJECT_SIPX_SOCKET_WINDOWS_H
#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")

#include "socket_base.h"
#include <winsock2.h>
#include <ws2tcpip.h>

class WindowsSocket : public Socket {
public:
    WindowsSocket();
    ~WindowsSocket() override;

    bool create_udp() override;
    bool create_tcp() override;
    bool bind(uint16_t port) override;
    bool connect(const char* dest_ip, uint16_t dest_port) override;
    int send(const void* data, size_t len) override;
    int recv(void* buffer, size_t max_len) override;
    int send_to(const void* data, size_t len, const char* dest_ip, uint16_t dest_port) override;
    int recv_from(void* buffer, size_t max_len, char* source_ip, uint16_t* source_port) override;
    void close() override;

    int get_last_error() override { return m_error; }

private:
    SOCKET m_socket = INVALID_SOCKET;
    sockaddr_in m_addr{};
    int m_error = 0;
};

#endif
#endif //PROJECT_SIPX_SOCKET_WINDOWS_H