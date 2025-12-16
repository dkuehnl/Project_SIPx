//
// Created by dkueh on 15.12.2025.
//

#include "SIPTimer.h"

#include <iostream>

SIPTimer::SIPTimer(EventDispatcher& disp, SIPLogWriter* log)
    : m_disp(disp), m_log(log), m_thread(&SIPTimer::run, this)
{}

SIPTimer::~SIPTimer() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
    }
    m_cv.notify_one();
    m_thread.join();
}

TimerID SIPTimer::schedule_at(const std::chrono::steady_clock::time_point time, Event evt) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const TimerID id = m_next_id++;
    m_jobs.push(TimerJob{time, id, std::move(evt)});
    m_cv.notify_one();
    return id;
}

TimerID SIPTimer::schedule_after(const std::chrono::milliseconds delay, Event evt) {
    return schedule_at(
        std::chrono::steady_clock::now() + delay,
        std::move(evt));
}

void SIPTimer::cancel(TimerID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::priority_queue<TimerJob, std::vector<TimerJob>, Compare> tmp;

    while (!m_jobs.empty()) {
        auto job = m_jobs.top();
        m_jobs.pop();
        if (job.id != id) {
            tmp.push(std::move(job));
        }
    }

    m_jobs = std::move(tmp);
}

void SIPTimer::run() {
    m_disp.register_listener(this);
    std::unique_lock<std::mutex> lock(m_mutex);

    while (m_running) {
        if (m_jobs.empty()) {
            m_cv.wait(lock);
            continue;
        }

        auto next_time = m_jobs.top().fire_at;
        if (m_cv.wait_until(lock, next_time) == std::cv_status::timeout) {
            auto job = m_jobs.top();
            m_jobs.pop();

            lock.unlock();
            m_disp.dispatch(job.evt);
            lock.lock();
        }
    }
}

void SIPTimer::on_event(const Event& evt) {

}