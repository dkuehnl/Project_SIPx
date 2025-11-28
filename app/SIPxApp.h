//
// Created by dkueh on 21.11.2025.
//

#ifndef PROJECT_SIPX_SIPXAPP_H
#define PROJECT_SIPX_SIPXAPP_H

#include "../parser/SIPParser.h"
#include "../logwriter/SIPLogWriter.h"
#include "../eventhandler/EventDispatcher.h"

struct ModuleFlags {
    bool network = false;
    bool parser = false;
    bool call = false;
    bool reg = false;
    bool media = false;
    bool logwriter = false;
};

class SIPxApp : public EventHandler {
public:
    explicit SIPxApp(ModuleFlags flags, const std::string& filepath = "");

    EventDispatcher m_dispatcher;
    void parse_raw_message(std::string&& raw_message) const;
    void on_event(const Event& evt) override;

    [[nodiscard]] const std::vector<std::unique_ptr<SIPMessage>>& get_messages() const { return m_msgs; }

private:
    ModuleFlags m_flags;
    std::unique_ptr<SIPLogWriter> m_logger;
    std::unique_ptr<SIPParser> m_parser;
    //std::unique_ptr<SIPNetwork> m_network;
    //std::unique_ptr<SIPMedia> m_media;
    //std::unique_ptr<SIPReg> m_reg;
    //std::unique_ptr<SIPCall> m_call;

    std::vector<std::unique_ptr<SIPMessage>> m_msgs;
};

#endif //PROJECT_SIPX_SIPXAPP_H