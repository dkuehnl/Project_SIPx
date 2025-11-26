//
// Created by dkueh on 21.11.2025.
//

#include "SIPxApp.h"

#include <iostream>

SIPxApp::SIPxApp(const ModuleFlags flags, const std::string& filepath)
    : m_flags(flags) {
    if (m_flags.logwriter) {
        if (filepath.empty()) {
            std::cerr << "Invalid instantiation, LogWriter activated, but no filepath provided." << std::endl;
            return;
        }
        m_logger = std::make_unique<SIPLogWriter>(filepath);
    }
    SIPLogWriter* logger = m_logger.get();

    if (m_flags.parser) {
        m_parser = std::make_unique<SIPParser>(logger);
    }
}

