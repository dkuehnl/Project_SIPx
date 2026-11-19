//
// Created by dkueh on 19.11.2026.
//

#ifndef PROJECT_SIPX_ILOGWRITER_H
#define PROJECT_SIPX_ILOGWRITER_H

#include <string_view>

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical
};

class ILogWriter {
public:
    virtual ~ILogWriter() = default;
    virtual bool is_enabled(LogLevel lvl) const noexcept = 0;
    virtual void log(LogLevel lvl, std::string_view source_module, std::string_view msg) = 0;
};

class NullLogger final : public ILogWriter {
public:
    bool is_enabled(LogLevel lvl) const noexcept override { return false; }
    void log(LogLevel, std::string_view source_module, std::string_view msg) override {}
    static NullLogger& instance() { static NullLogger n; return n; }
};
#endif //PROJECT_SIPX_ILOGWRITER_H
