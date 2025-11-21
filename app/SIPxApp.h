//
// Created by dkueh on 21.11.2025.
//

#ifndef PROJECT_SIPX_SIPXAPP_H
#define PROJECT_SIPX_SIPXAPP_H

#include "../parser/SIPMessage.h"
#include "../logwriter/SIPLogWriter.h"

struct ModuleFlags {
    bool network = false;
    bool parser = false;
    bool call = false;
    bool reg = false;
    bool media = false;
    bool logwriter = false;
};

class SIPxApp {
public:
    explicit SIPxApp(ModuleFlags flags);

private:
    ModuleFlags m_flags;
    SIPLogWriter m_logger;

};


#endif //PROJECT_SIPX_SIPXAPP_H