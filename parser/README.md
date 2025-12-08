# Parser

## Overview

The SIP Parser module provides the parsing backbone for Project SIPx.\
It is responsible for taking a raw SIP message string and turning it
into a structured C++ object (`SIPMessage` and its derived types
`SIPRequest` / `SIPResponse`).\
This module focuses solely on parsing logic and avoids imposing any SIP
behavior or call-flow semantics. It is designed to be **minimal**,
**predictable**, and **fully controllable** by the caller.

You can use this module:

-   **as part of the full SIPxApp**
-   **standalone**, without the app
-   **with or without** logging
-   **with or without** event dispatching

Everything is optional and loosely coupled.

------------------------------------------------------------------------

## Architecture

The parsing system consists of two primary classes:

### 1. `SIPMessage`

A detailed parsing engine for SIP messages.\
This class:

-   parses request line or status line\
-   extracts core SIP headers (From, To, CSeq, Contact, Via, Content,
    etc.)\
-   splits and evaluates list-based headers (Supported, Require, Allow)\
-   parses Session-Timer information (Min-SE, Session-Expires,
    refresher)\
-   optionally logs parsing steps if a `SIPLogWriter` is attached\
-   optionally receives events via `EventHandler`\
-   exposes all parsed components through lightweight getters
    (`std::string_view`)

Derived classes:

-   `SIPRequest`
-   `SIPResponse`

### 2. `SIPParser`

A wrapper used as the main entry point for users.\
It accepts raw SIP strings and automatically decides:

-   Is this a **response**?\
-   Is this a **request**?

Based on that, it creates the corresponding derived class and returns it
as a `std::unique_ptr<SIPMessage>`.

It also:

-   optionally logs every parsed message\
-   optionally emits and listens to events\
-   keeps a simple counter of handled packets

------------------------------------------------------------------------

## Message Flow

1.  Raw SIP string is passed into `SIPParser::parse_message()`.
2.  Parser evaluates the first line to decide between request/response.
3.  The correct derived `SIPMessage` type is instantiated.
4.  The message is parsed.
5.  Optional: logs are written.
6.  Optional: events are dispatched.
7.  The parsed message is returned.

------------------------------------------------------------------------

## Optional Components

### Logging (`SIPLogWriter`)

If provided: parsing messages, errors, malformed headers get logged.\
If not provided: silent parsing.

### Event Dispatching

Events like:

-   `RAW_MESSAGE_RECEIVED`
-   `MESSAGE_PARSED`
-   `LOG_MESSAGE`

can be emitted. Fully optional.

------------------------------------------------------------------------

## Basic Usage Example

``` cpp
#include "parser/SIPParser.h"

SIPParser parser(nullptr, nullptr);  // no logging, no events

std::string raw =
    "INVITE sip:alice@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP host;branch=z9hG4bK123\r\n"
    "From: <sip:bob@example.com>;tag=111\r\n"
    "To: <sip:alice@example.com>\r\n"
    "CSeq: 1 INVITE\r\n"
    "Contact: <sip:bob@host>\r\n"
    "Content-Length: 0\r\n\r\n";

auto msg = parser.parse_message(std::move(raw));

std::cout << "CSeq-Type: " << msg->get_cseq_typ() << "\n";
```

------------------------------------------------------------------------

## File Overview

### `SIPParser.h/.cpp`

-   Entry point\
-   Creates correct SIP message type\
-   Handles logging and events\
-   Maintains packet counter

### `SIPMessage.h/.cpp`

-   Full SIP parsing implementation\
-   Header-specific parsing (Via, Contact, CSeq, etc.)\
-   Getter-based access to parsed content

------------------------------------------------------------------------

## Status

  Component          Status
  ------------------ --------------
  SIPMessage         ✔️ completed
  SIPParser          ✔️ completed
  Request/Response   ✔️ completed
  Logging            ✔️ optional
  Event handling     ✔️ optional
