//
// Created by dkueh on 03.11.2025.
//

#include "SIPMessage.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <utility>
#include <chrono>

SIPMessage::SIPMessage(std::string message, std::string source_ip, std::uint16_t source_port)
    : m_message(std::move(message)), m_source_ip(std::move(source_ip)), m_source_port(source_port) {

    if (m_message.empty()) {
        throw std::invalid_argument("[SIPMessage.cpp] Message is empty");
    }
    if (m_source_ip.empty()) {
        throw std::invalid_argument("[SIPMessage.cpp] No Source-IP detected");
    }
    if (m_source_port == 0) {
        throw std::invalid_argument("[SIPMessage.cpp] Invalid Source-Port detected");
    }

    if (m_message.find("\r\n\r\n") == std::string::npos) {
        throw std::invalid_argument("[SIPMessage.cpp] Message does not end with \r\n\r");
    }


    auto start = std::chrono::high_resolution_clock::now();
    SIPMessage::parse_message();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Execution-Time: " << duration.count() << "ms" << std::endl;
}


void SIPMessage::parse_message() {
    const char* ptr = m_message.data();
    const char* end = ptr + m_message.size();

    const char* line_end = std::search(ptr, end, "\r\n", "\r\n" + 2);
    if (line_end != end) {
        m_request_line = std::string_view(ptr, line_end - ptr);
        ptr = line_end + 2;
    }

    while (ptr < end) {
        const char* next_line = std::search(ptr, end, "\r\n", "\r\n"+2);
        if (next_line == ptr || next_line == end) break;

        const char* colon = std::find(ptr, next_line, ':');
        if (colon != next_line) {
            std::string_view name(ptr, colon - ptr);
            std::string_view value(colon + 1, next_line - colon);

            while (!value.empty() && value.front() == ' ') value.remove_prefix(1);

            auto cmp = [](std::string_view a, std::string_view b) {
                if (a.size() != b.size()) return false;
                for (size_t i = 0; i < a.size(); ++i) {
                    if (std::tolower(a[i]) != std::tolower(b[i])) return false;
                }
                return true;
            };

            if (cmp(name, "From")) from_header = value;
            else if (cmp(name, "To")) to_header = value;
            else if (cmp(name, "CSeq")) cseq_header = value;
            else if (cmp(name, "Content-Length")) content_length_header = value;
            else m_headers[name] = value;
        }
        ptr = next_line + 2;
    }
}

std::string_view SIPMessage::get_header(const std::string_view header_value) const {
    auto element = m_headers.find(header_value);
    if (element != m_headers.end()) {
        return element->second;
    }
    return "not available";
}

