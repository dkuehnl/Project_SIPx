//
// Created by dkueh on 16.12.2025.
//

#ifndef PROJECT_SIPX_UTILS_H
#define PROJECT_SIPX_UTILS_H

#include <string_view>
#include <string>

namespace utils::sip {
    inline bool is_response(const std::string_view sip_message) {
        const auto first_line_end = sip_message.find("\r\n");
        if (const std::string_view first_line = sip_message.substr(0, first_line_end); first_line.starts_with("SIP/2.0")) {
            return false;
        }
        return true;
    }

    inline std::string build_new_from_tag() {
        std::string from_tag{};

        return from_tag;
    }

    inline std::string build_new_to_tag() {
        std::string to_tag{};

        return to_tag;
    }

    inline std::string build_new_call_id() {
        std::string call_id{};

        return call_id;
    }

    inline std::string build_new_branch() {
        std::string branch{};

        return branch;
    }
}

#endif //PROJECT_SIPX_UTILS_H