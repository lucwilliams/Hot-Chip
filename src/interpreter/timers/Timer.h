#pragma once

#include <cstdint>
#include <chrono>
#include <ctime>
#include <thread>

class Timer {
    private:
        // Require derived timer classes to implement tickTimer method
        virtual void tickTimer(std::stop_token stopToken) = 0;

    protected:
        // Use join thread for safety
        std::jthread m_timerThread;
        uint8_t m_timer {0};

    public:
        inline void setTimer(uint8_t value) {
            m_timer = value;
        }
};
