/*
 *  Project SIPx
 *  -------------
 *  SIPx is a flexible and extensible SIP engine designed primarily for
 *  troubleshooting, edge-case testing, and IMS-related scenarios. Its main goal
 *  is to give developers and DevOps engineers the ability to inspect,
 *  manipulate, and stress-test SIP/SDP/RTP flows in detail.
 *
 *  Description of SIPNetwork:
 *  ---------------------------
 *  The SIPNetwork module is responsible for all low-level network communication
 *  within the SIPx project. It encapsulates socket handling, send/receive logic,
 *  and platform-specific networking details behind an event-driven interface.
 *
 *  SIPNetwork operates as an asynchronous network worker that integrates tightly
 *  with the EventDispatcher / EventHandler system of the project. All interaction
 *  with the module (socket creation, sending messages, shutdown, etc.) is done
 *  exclusively via events.
 *
 *  Main responsibilities:
 *    • Creating and managing UDP and TCP sockets on demand
 *    • Sending SIP messages over UDP or TCP
 *    • Receiving SIP messages asynchronously
 *    • Reassembling fragmented TCP SIP messages based on SIP headers
 *    • Dispatching fully received raw SIP messages as events
 *    • Abstracting platform-specific socket handling (e.g. Winsock on Windows)
 *
 *  Internal architecture:
 *    • Runs its own worker thread (network_loop)
 *    • Uses non-blocking sockets and polling (WSAPoll on Windows)
 *    • Maintains a thread-safe socket registry (SocketEntry objects)
 *    • Uses an internal event queue to decouple event reception from processing
 *
 *  Socket management:
 *    • Each socket is stored in a SocketEntry structure containing:
 *        - The socket instance
 *        - Transport type (TCP or UDP)
 *        - Receive buffer (used for TCP stream reassembly)
 *        - Destination IP and port (for UDP and connected TCP)
 *    • Sockets are indexed by their native socket handle
 *
 *  TCP specifics:
 *    • TCP is treated as a byte stream
 *    • Incoming data is buffered until a complete SIP message is available
 *    • SIP message boundaries are detected via:
 *        - Header termination (\r\n\r\n)
 *        - Optional Content-Length header
 *    • Multiple SIP messages may be extracted from a single receive buffer
 *
 *  UDP specifics:
 *    • Each received datagram is treated as a complete SIP message
 *    • No buffering or reassembly is required
 *
 *  Event integration:
 *    • SIPNetwork registers itself as an EventHandler
 *    • Incoming events are queued and processed inside the network thread
 *    • Outgoing events include:
 *        - NETWORK_SOCKET_CREATED
 *        - RAW_MESSAGE_RECEIVED
 *        - LOG_MESSAGE
 *
 *  Threading model:
 *    • One dedicated network thread handles:
 *        - Event processing
 *        - Polling sockets for readability
 *        - Receiving and dispatching messages
 *    • Mutexes protect:
 *        - Internal event queue
 *        - Socket registry
 *
 *  Logging:
 *    • Logging is optional
 *    • If a SIPLogWriter is provided, log entries are written directly
 *    • Log messages are additionally dispatched as LOG_MESSAGE events
 *
 *  Purpose and usage:
 *    The SIPNetwork module acts as the single networking backend for SIPx.
 *    Higher-level components never interact with sockets directly but instead
 *    communicate exclusively via events, making the overall architecture
 *    portable, testable, and extensible.
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
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */



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

class SIPNetwork final : public EventHandler {
public:
    explicit SIPNetwork(EventDispatcher* disp = nullptr, SIPLogWriter* log = nullptr);
    ~SIPNetwork() override;


    void on_event(Event& evt) override;
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