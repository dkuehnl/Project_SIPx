//
// Created by dkueh on 08.12.2025.
//

#ifndef PROJECT_SIPX_SOCKET_BASE_H
#define PROJECT_SIPX_SOCKET_BASE_H
#include <cstdint>

class Socket {
public:
    virtual ~Socket() = default;

    virtual bool create_udp() = 0;
    virtual bool create_tcp() = 0;
    virtual bool bind(uint16_t port) = 0;
    virtual bool connect(const char* dest_ip, uint16_t dest_port) = 0;
    virtual int send(const void* data, size_t len) = 0;
    virtual int recv(void* buffer, size_t max_len) = 0;
    virtual int send_to(const void* data, size_t len, const char* dest_ip, uint16_t dest_port) = 0;
    virtual int recv_from(void* buffer, size_t max_len, char* source_ip, uint16_t* source_port) = 0;
    virtual void close() = 0;
    virtual int get_last_error() const = 0;
    virtual intptr_t get_handle() const = 0;
};

#endif //PROJECT_SIPX_SOCKET_BASE_H