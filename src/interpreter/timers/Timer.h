#pragma once

#include <cstdint>
#include <thread>

class Timer {
    virtual void tickTimer(std::stop_token stopToken) = 0;

    protected:
        // Use join thread for safety
        std::jthread m_timerThread;
        std::atomic<uint8_t> m_timer {0};

    public:
        void setTimer(uint8_t value) {
            m_timer = value;
        }

        virtual void reset() = 0;
        virtual ~Timer() = default;
};
