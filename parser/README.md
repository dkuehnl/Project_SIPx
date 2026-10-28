# Parser

Turns a raw SIP message into a plain `SIPMessage` struct — requests and responses alike.

The parser only **extracts text**. It does not convert values into numbers, does not evaluate SIP semantics and never modifies or repairs the input. What was on the wire is what ends up in the struct — including duplicates and malformed headers — so higher-level modules can decide how to deal with non-RFC-compliant messages.

---

## Design Principles

- **Text only** — all values are stored as received; numeric conversion is done by the module that needs the value.
- **Nothing gets lost** — every header line ends up in exactly one place (see [Header Handling](#header-handling)).
- **No evaluation** — a missing, empty or malformed header simply leaves its field empty.
- **Self-contained result** — `SIPMessage` holds owned copies; the input buffer can be released after parsing.
- **Framing is the caller's job** — `parse_message()` expects exactly one complete message.
- **Loosely coupled** — usable standalone or within the SIPxApp, with or without logging and event dispatching.

---

## Architecture

```
raw message ─► SIPParser::parse_message() ─► start-line ─► header lines ─► dispatch ─► SIPMessage
                                                                    │
                                                  known-header table (constexpr)
                                                  + bitset for duplicate detection
```

| Component | Responsibility |
|---|---|
| `SIPParser` | Entry point. Parses start-line, header lines and body; optionally reacts to `RAW_MESSAGE_RECEIVED` and dispatches `MESSAGE_PARSED`. |
| `SIPMessage` | Plain data struct without logic or getters. Holds start-line data, known headers in dedicated fields and all remaining headers as raw `Header` entries. |

Known headers are described in a single `constexpr` table (name, compact form, handler, list flag). Dispatching and duplicate detection are done centrally; the handlers only decompose values.

---

## Header Handling

| Case | Field | `malformed_header` | `duplicate_header` | `unknown_header` |
|---|---|---|---|---|
| Known header, single occurrence | parsed | – | – | – |
| Known header, cannot be decomposed | empty | raw value | – | – |
| Single-value header, repeated | first occurrence | first, if malformed | every further occurrence (unparsed) | – |
| List header, multiple lines / values | all valid elements | each malformed element | – | – |
| Missing or empty header | empty | – | – | – |
| Unknown header (also repeated) | – | – | – | raw entry |

- The **first occurrence** of a single-value header occupies its field — even if it is malformed. Further occurrences are never parsed.
- Long and compact form (`Call-ID` / `i`) count as the same header; matching is case-insensitive.
- Header names in `unknown_header`, `malformed_header` and `duplicate_header` are stored **as received**.

### Known Headers

| Header | Compact | Type | Field |
|---|---|---|---|
| Call-ID | `i` | single | `call_id` |
| Content-Length | `l` | single | `content_length` |
| Content-Type | `c` | single | `content_type` |
| Max-Forwards | – | single | `max_forwards` |
| From | `f` | single | `from` (`NameAddr`) |
| To | `t` | single | `to` (`NameAddr`) |
| CSeq | – | single | `cseq` (`CSeq`) |
| Via | `v` | list | `via` (`std::vector<Via>`) |
| Contact | `m` | list | `contacts` (`std::vector<Contact>`) |
| Route | – | list | `route` (raw entries) |
| Record-Route | – | list | `record_route` (raw entries) |
| Allow | – | list | `allow` |
| Supported | `k` | list | `supported` |
| Require | – | list | `require` |

A header counts as **malformed** only if it cannot be decomposed structurally — e.g. From/To/Contact without a parsable URI or with an unclosed `<` / `"`, CSeq without separator, Via without transport/host separator or with an unclosed `[`. Plain string headers (Call-ID, Content-Length, Content-Type, Max-Forwards) and Route entries are never malformed.

---

## Usage

```cpp
#include "parser/SIPParser.h"

SIPParser parser;   // no dispatcher, no logging

constexpr std::string_view raw =
    "INVITE sip:bob@b.example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP pc1.a.example.com:5060;branch=z9hG4bK1\r\n"
    "From: <sip:alice@a.example.com>;tag=1\r\n"
    "To: <sip:bob@b.example.com>\r\n"
    "Call-ID: abc@a.example.com\r\n"
    "CSeq: 1 INVITE\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

const SIPMessage msg = parser.parse_message(raw);

if (!msg.is_complete) { /* no header terminator found */ }

std::cout << msg.cseq.method << '\n';           // "INVITE"
for (const auto& h : msg.duplicate_header) {    // unexpected repetitions
    std::cout << h.name << ": " << h.value << '\n';
}
```

### Event Integration

If an `EventDispatcher` is passed to the constructor, the parser registers itself as listener, parses `RAW_MESSAGE_RECEIVED` events and dispatches `MESSAGE_PARSED`. Dispatcher and `SIPLogWriter` are optional and not owned — the caller keeps them alive for the lifetime of the parser.

---

## Files

| File | Content |
|---|---|
| `SIPParser.h/.cpp` | Parser class, known-header table, decomposition helpers |
| `SIPMessage.h` | `SIPMessage` and sub-structs (`NameAddr`, `Via`, `CSeq`, `Contact`, `Header`) |

---

## Limitations

- Line folding (continuation lines starting with SP/HTAB) is not supported yet.
