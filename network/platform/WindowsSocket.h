/*
 *  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. Its main goal
 *  is to give developers and DevOps engineers the ability to inspect,
 *  manipulate, and stress-test SIP/SDP/RTP flows in detail, including realistic
 *  failure and corner-case simulations.
 *
 *  Description of WindowsSocket:
 *  ------------------------------
 *  The WindowsSocket class is the Windows-specific implementation of the
 *  generic Socket interface defined in socket_base.h.
 *
 *  It wraps the Winsock2 API and translates the platform-independent socket
 *  operations into native Windows socket calls.
 *
 *  This class is only compiled on Windows platforms and is guarded by
 *  conditional compilation macros.
 *
 *  Responsibilities:
 *    • Encapsulate Winsock socket creation and cleanup
 *    • Implement UDP and TCP communication using Winsock2
 *    • Translate Winsock error states into a uniform error interface
 *    • Expose the native SOCKET handle for polling and identification
 *
 *  Platform specifics:
 *    • Uses Winsock2 (ws2_32.lib)
 *    • SOCKET handles are internally stored and exposed as intptr_t
 *    • Error codes are retrieved via WSAGetLastError()
 *
 *  Design notes:
 *    • This class does not manage WSAStartup / WSACleanup
 *      (this is handled by higher-level modules such as SIPNetwork)
 *    • All socket operations are thin wrappers around the corresponding
 *      Winsock calls
 *    • No protocol-specific logic (e.g. SIP parsing) is implemented here
 *
 *  Threading:
 *    • The class itself is not thread-safe
 *    • Synchronization is expected to be handled by the caller
 *
 *  Usage:
 *    • Typically instantiated and managed by SIPNetwork
 *    • Accessed exclusively through the Socket interface
 *
 *  Author:  Dennis Kühnlein
 *  Version: v0.1
 *  Created: 09.12.2025
 *  Updated: 15.12.2025
 *
 *  License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */

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

    int get_last_error() const override { return m_error; }
    intptr_t get_handle() const override { return static_cast<intptr_t>(m_socket); }

private:
    SOCKET m_socket = INVALID_SOCKET;
    sockaddr_in m_addr{};
    int m_error = 0;
};

#endif
#endif //PROJECT_SIPX_SOCKET_WINDOWS_H