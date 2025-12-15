/*
 *  Project SIPx
 *  -------------
*  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. Its main goal
 *  is to give developers and DevOps engineers the ability to inspect,
 *  manipulate, and stress-test SIP/SDP/RTP flows in detail, including realistic
 *  failure and corner-case simulations.
 *
 *  Description of socket_base:
 *  ----------------------------
 *  The socket_base interface defines the platform-independent abstraction
 *  for all network socket operations used within the SIPx project.
 *
 *  It provides a minimal and uniform API for UDP and TCP communication,
 *  independent of the underlying operating system or socket implementation.
 *  Concrete implementations (e.g. WindowsSocket, UnixSocket) are expected
 *  to translate these calls to the respective platform APIs.
 *
 *  This abstraction allows higher-level components (such as SIPNetwork)
 *  to operate entirely without platform-specific code, enabling portability
 *  and cleaner separation of concerns.
 *
 *  Design goals:
 *    • Provide a minimal but complete socket API for SIP usage
 *    • Hide platform-specific socket types and error handling
 *    • Support both UDP and TCP transports
 *    • Allow non-blocking and event-driven usage patterns
 *
 *  Interface overview:
 *    • Socket creation:
 *        - create_udp()
 *        - create_tcp()
 *    • Connection and binding:
 *        - bind()
 *        - connect()
 *    • Data transmission:
 *        - send() / recv()           (TCP)
 *        - send_to() / recv_from()   (UDP)
 *    • Lifecycle and diagnostics:
 *        - close()
 *        - get_last_error()
 *        - get_handle()
 *
 *  Error handling:
 *    • All methods return either a boolean (success/failure)
 *      or an integer byte count
 *    • On failure, get_last_error() returns the last platform-specific
 *      socket error code
 *
 *  Socket handles:
 *    • get_handle() exposes the native socket handle as an intptr_t
 *    • This is primarily used for polling and socket identification
 *      inside the SIPNetwork module
 *
 *  Usage pattern:
 *    • Higher-level modules interact exclusively with this interface
 *    • Concrete socket implementations are created via platform-specific
 *      factories or conditional compilation
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