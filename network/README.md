# Network

## Overview

The `network` module is the low-level networking backend of the SIPx application.
It is responsible for all socket-based communication, including sending and
receiving SIP messages over UDP and TCP.

This module intentionally contains **no SIP parsing or protocol logic**.
Its sole responsibility is reliable, asynchronous transport of raw SIP messages
and the abstraction of platform-specific socket APIs.

All interaction with this module is event-driven.

---

## Responsibilities

The network module is responsible for:

- Creating and managing UDP and TCP sockets
- Abstracting platform-specific socket implementations
- Sending raw SIP messages over UDP or TCP
- Receiving SIP messages asynchronously
- Reassembling fragmented TCP SIP messages
- Dispatching fully received SIP messages as events
- Providing a single networking backend for the entire application

It explicitly does **not**:

- Parse SIP headers or bodies
- Interpret SIP semantics
- Manage SIP dialogs or transactions
- Handle RTP/RTCP media streams (planned separately)

---

## High-Level Architecture

```
+-------------------+
|   SIPx Modules    |
| (Parser, Logic,  |
|  Application)     |
+---------+---------+
          |
          | Events
          v
+-------------------+
|   SIPNetwork      |
|-------------------|
| - EventHandler    |
| - Network Thread  |
| - Socket Registry |
+---------+---------+
          |
          | Socket Interface
          v
+-------------------+
|   Socket (API)    |  <- socket_base.h
+---------+---------+
          |
          | Platform-specific
          v
+-------------------+
| WindowsSocket     |
| (Winsock2)        |
+-------------------+
```

---

## Event-Driven Design

The network module integrates tightly with the global `EventDispatcher`.

### Incoming events (handled by SIPNetwork)

Typical incoming events include:

- `CREATE_NETWORK_SOCKET`
- `SEND_MESSAGE`
- `SEND_MEDIA` (reserved / future use)

These events are **queued internally** and processed inside the network thread.
This ensures that all socket operations happen in a single, controlled context.

### Outgoing events (dispatched by SIPNetwork)

The module emits events such as:

- `NETWORK_SOCKET_CREATED`
- `RAW_MESSAGE_RECEIVED`
- `LOG_MESSAGE`

Higher-level modules never interact with sockets directly.
They only react to events.

---

## Threading Model

The module runs **one dedicated network thread**.

Responsibilities of the network thread:

- Processing queued internal events
- Polling sockets for incoming data
- Receiving network data
- Reassembling TCP streams
- Dispatching received messages as events

Thread safety is ensured by:

- A mutex-protected event queue
- A mutex-protected socket registry
- No socket access outside the network thread

---

## Socket Abstraction Layer

### socket_base.h

`socket_base.h` defines the **platform-independent socket interface** used
throughout the SIPx project.

Key characteristics:

- Supports both UDP and TCP
- Minimal API focused on SIP requirements
- Exposes native socket handles for polling
- Provides uniform error access via `get_last_error()`

Higher-level modules only depend on this interface.

---

## Windows Implementation

### WindowsSocket

The `WindowsSocket` class is the Windows-specific implementation of the
generic `Socket` interface.

Implementation details:

- Uses Winsock2 (`ws2_32`)
- Wraps `socket`, `bind`, `connect`, `send`, `recv`, `sendto`, `recvfrom`
- Stores Winsock error codes internally
- Exposes the native `SOCKET` handle for polling

`WSAStartup()` and `WSACleanup()` are handled by the `SIPNetwork` module,
not by the socket class itself.

---

## TCP vs UDP Handling

### UDP

- Each received datagram is treated as a complete SIP message
- No buffering or reassembly
- Source IP and port are available via `recv_from()`

### TCP

- TCP is treated as a byte stream
- Incoming data is appended to a receive buffer
- SIP message boundaries are detected using:
  - `\r\n\r\n` (end of SIP headers)
  - Optional `Content-Length` header
- Multiple SIP messages may be extracted from a single buffer
- Partial messages are kept until fully received

This logic ensures correct handling of:
- Fragmented TCP packets
- Multiple SIP messages in a single TCP read
- Slow or bursty TCP connections

---

## Logging

Logging is optional and dual-path:

- Log messages are dispatched as `LOG_MESSAGE` events
- If a `SIPLogWriter` is provided, logs are written directly as well

This allows:
- Centralized logging via events
- Optional file-based or persistent logging

---

## Typical Usage Flow

1. Application dispatches `CREATE_NETWORK_SOCKET`
2. SIPNetwork creates the socket (UDP or TCP)
3. SIPNetwork dispatches `NETWORK_SOCKET_CREATED`
4. Application dispatches `SEND_MESSAGE`
5. SIPNetwork sends data via the socket
6. Incoming network data is received asynchronously
7. SIPNetwork dispatches `RAW_MESSAGE_RECEIVED`
8. Higher-level modules parse and process the SIP message

---

## Design Goals

- Strict separation of concerns
- Platform-independent networking
- Event-driven, asynchronous design
- Safe TCP stream handling
- Minimal and predictable socket abstraction
- Easy extension to additional platforms (e.g. Unix)

---

## Future Extensions

Planned or possible extensions include:

- Unix / Linux socket implementation
- IPv6 support
- Socket option configuration (timeouts, reuse, non-blocking flags)
- TLS support (via additional socket abstraction)
- RTP / media socket handling

---

## Summary

The `network` module is the foundation of all SIP communication in SIPx.
It provides a clean, event-driven abstraction over platform-specific sockets
and ensures that higher-level modules can focus entirely on SIP logic without
needing to care about networking details.

If you understand this module, you understand how all data enters and leaves
the SIPx application.
