//
// Created by dkueh on 21.11.2025.
//

#ifndef PROJECT_SIPX_SIPXAPP_H
#define PROJECT_SIPX_SIPXAPP_H

#include <any>

#include "../parser/SIPParser.h"
#include "../logwriter/SIPLogWriter.h"

struct ModuleFlags {
    bool network = false;
    bool parser = false;
    bool call = false;
    bool reg = false;
    bool media = false;
    bool logwriter = false;
};

enum class EventType {
    NETWORK_SOCKET_CREATED,
    NETWORK_SOCKET_CLOSED,
    RAW_MESSAGE_RECEIVED,
    MESSAGE_PARSED,
    SEND_MESSAGE,
    TIME_TO_REGISTER,
    INITIATE_CALL,
    START_MEDIA,
    SEND_MEDIA,
    RECEIVED_MEDIA
};

struct Event {
    EventType type;
    std::any data;
};

class SIPxApp {
public:
    explicit SIPxApp(ModuleFlags flags, const std::string& filepath = "");

private:
    ModuleFlags m_flags;
    std::unique_ptr<SIPLogWriter> m_logger;
    std::unique_ptr<SIPParser> m_parser;
    //SIPNetwork m_network;
    //SIPMedia m_media;
    //SIPReg m_reg;
    //SIPCall m_call;
};

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void onEvent(const Event& evt) = 0;
};

class EventDispatcher {
public:
    std::vector<IEventHandler*> listeners;
    void registerListener(IEventHandler* l) {
        listeners.push_back(l);
    }

    void dispatch(const Event& evt) const {
        for (auto* l : listeners) {
            l->onEvent(evt);
        }
    }
};

#endif //PROJECT_SIPX_SIPXAPP_H