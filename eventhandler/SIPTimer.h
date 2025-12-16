//
// Created by dkueh on 15.12.2025.
//

#ifndef PROJECT_SIPX_SIPTIMER_H
#define PROJECT_SIPX_SIPTIMER_H

#include "EventDispatcher.h"
#include "EventHandler.h"
#include "../logwriter/SIPLogWriter.h"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>

using TimerID = uint64_t;

class SIPTimer final : public EventHandler {
public:
    explicit SIPTimer(EventDispatcher& disp, SIPLogWriter* log = nullptr);
    ~SIPTimer() override;

    void on_event(const Event& evt) override;

    TimerID schedule_at(std::chrono::steady_clock::time_point time, Event evt);
    TimerID schedule_after(std::chrono::milliseconds delay, Event evt);
    void cancel(TimerID id);

private:
    struct TimerJob {
        std::chrono::steady_clock::time_point fire_at;
        TimerID id;
        Event evt;
    };
    struct Compare {
        bool operator()(const TimerJob& a, const TimerJob& b) const {
            return a.fire_at > b.fire_at;
        }
    };
    void run();

    EventDispatcher& m_disp;
    SIPLogWriter* m_log;
    std::priority_queue<TimerJob, std::vector<TimerJob>, Compare> m_jobs;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_running{true};
    std::thread m_thread;
    TimerID m_next_id{1};
};


#endif //PROJECT_SIPX_SIPTIMER_H