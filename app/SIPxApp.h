//
// Created by dkueh on 21.11.2025.
//

#ifndef PROJECT_SIPX_SIPXAPP_H
#define PROJECT_SIPX_SIPXAPP_H

#include "../parser/SIPParser.h"
#include "../logwriter/SIPLogWriter.h"
#include "../eventhandler/EventDispatcher.h"
#include "../network/SIPNetwork.h"

struct SIPConnection {
    /*** network-settings ***/
    intptr_t socket_id;
    bool tcp = true;
    std::string fqdn;

    /*** dns-settings ***/
    bool is_resolved = false;
    std::string prio10;
    std::string prio20;
    std::string prio30;

    /*** register-state ***/
    bool registered = false;
    std::string registered_uri {};
    std::string password {};
    std::string domain {};
    int16_t reg_expire = 0;

    /*** call-state ***/
    bool call_active = false;
};

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

    /*** Network-related functions (for own handled connections, does not store the data) +***/
    void establish_tcp_connection(std::string& dest_ip, uint16_t dest_port, uint16_t src_port = 5060) const;
    void establish_udp_connection(std::string& dest_ip, uint16_t dest_port, uint16_t src_port = 5060) const;

    /*** setup functions ***/
    void set_network_settings(bool use_tcp, std::string& dest_ip, uint16_t dest_port, uint16_t src_port = 5060);
    void set_fqdn(std::string& fqdn);
    void set_register_settings(std::string& reg_number, std::string& password, std::string& domain, int16_t expire = 600)

    /*** register ***/
    ErrorCode register_sip(std::string& reg_number);


private:
    ModuleFlags m_flags;
    std::unique_ptr<SIPLogWriter> m_logger;
    std::unique_ptr<SIPParser> m_parser;
    std::unique_ptr<SIPNetwork> m_network;
    //std::unique_ptr<SIPMedia> m_media;
    //std::unique_ptr<SIPReg> m_reg;
    //std::unique_ptr<SIPCall> m_call;

    std::vector<std::unique_ptr<SIPMessage>> m_msgs;
    /*** map to store various sip-connections, key is the reg_number ***/
    std::unordered_map<std::string, std::vector<SIPConnection>> m_sip_connections;
};

#endif //PROJECT_SIPX_SIPXAPP_H