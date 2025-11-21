//
// Created by dkueh on 21.11.2025.
//

#ifndef PROJECT_SIPX_SIPPARSER_H
#define PROJECT_SIPX_SIPPARSER_H

#include <memory>

#include "SIPMessage.h"
#include "SIPRequest.h"
#include "SIPResponse.h"
#include "../logwriter/SIPLogWriter.h"

class SIPParser {
public:
    explicit SIPParser(SIPLogWriter* logger = nullptr);
    std::unique_ptr<SIPMessage> parse_message(std::string&& sip_message);
    ~SIPParser();

private:
    SIPLogWriter* m_logger;
    unsigned int m_paket_count = 0;

    static bool is_response(const std::string_view sip_message);
};


#endif //PROJECT_SIPX_SIPPARSER_H