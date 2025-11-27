//
// Created by dkueh on 27.11.2025.
//

#ifndef PROJECT_SIPX_EVENT_H
#define PROJECT_SIPX_EVENT_H

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
    std::string* raw_msg = nullptr;
    SIPMessage* parsed_msg = nullptr;
    mutable bool ownership_claimed = false;
};


#endif //PROJECT_SIPX_EVENT_H