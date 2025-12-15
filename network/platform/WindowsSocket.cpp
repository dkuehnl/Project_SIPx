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


#include "WindowsSocket.h"

WindowsSocket::WindowsSocket() = default;

WindowsSocket::~WindowsSocket() {
    WindowsSocket::close();
}

bool WindowsSocket::create_udp() {
    m_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET) {
        m_error = WSAGetLastError();
        return false;
    }
    m_error = 0;
    return true;
}

bool WindowsSocket::create_tcp() {
    m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        m_error = WSAGetLastError();
        return false;
    }
    m_error = 0;
    return true;
}

bool WindowsSocket::bind(const uint16_t port) {
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(m_socket, reinterpret_cast<sockaddr*>(&m_addr), sizeof(m_addr)) == SOCKET_ERROR) {
        m_error = WSAGetLastError();
        return false;
    }
    m_error = 0;
    return true;
}

bool WindowsSocket::connect(const char* dest_ip, const uint16_t dest_port) {
    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);

    if (::connect(m_socket, reinterpret_cast<sockaddr*>(&dest_addr), sizeof(dest_addr)) == SOCKET_ERROR) {
        m_error = WSAGetLastError();
        return false;
    }
    m_error = 0;
    return true;
}

/* for TCP */
int WindowsSocket::send(const void* data, const size_t len) {
    int sent = ::send(m_socket, static_cast<const char*>(data), static_cast<int>(len), 0);
    if (sent == SOCKET_ERROR) {
        m_error = WSAGetLastError();
        return -1;
    }
    m_error = 0;
    return sent;
}

int WindowsSocket::recv(void* buffer, const size_t max_len) {
    const int received = ::recv(m_socket, static_cast<char*>(buffer), static_cast<int>(max_len), 0);
    if (received == SOCKET_ERROR) {
        m_error = WSAGetLastError();
        return -1;
    }
    m_error = 0;
    return received;
}

/* for UDP */
int WindowsSocket::send_to(const void* data, const size_t len, const char* dest_ip, const uint16_t dest_port) {
    sockaddr_in target_addr{};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(dest_port);
    target_addr.sin_addr.s_addr = inet_addr(dest_ip);

    const int sent = ::sendto(m_socket, static_cast<const char*>(data), static_cast<int>(len), 0,
        reinterpret_cast<sockaddr*>(&target_addr), sizeof(target_addr));

    if (sent == SOCKET_ERROR) {
        m_error = WSAGetLastError();
        return -1;
    }
    m_error = 0;
    return sent;
}

int WindowsSocket::recv_from(void* buffer, const size_t max_len, char* source_ip, uint16_t* source_port) {
    sockaddr_in from_addr{};
    int addr_len = sizeof(from_addr);
    const int received = ::recvfrom(m_socket, static_cast<char*>(buffer), static_cast<int>(max_len), 0,
        reinterpret_cast<sockaddr*>(&from_addr), &addr_len);

    if (received == SOCKET_ERROR) {
        m_error = WSAGetLastError();
        return -1;
    }
    if (source_ip) {
        strcpy(source_ip, inet_ntoa(from_addr.sin_addr));
    }
    if (source_port) {
        *source_port = ntohs(from_addr.sin_port);
    }

    m_error = 0;
    return received;
}


void WindowsSocket::close() {
    if (m_socket != INVALID_SOCKET) {
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

