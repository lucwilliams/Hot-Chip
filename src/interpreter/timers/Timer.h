#pragma once

#include <cstdint>
#include <chrono>
#include <ctime>
#include <thread>

class Timer {
    // Use join thread for safety
    std::jthread timerThread;

    protected:
        uint8_t m_timer {0};

    public:
        explicit Timer();
        void setTimer(uint8_t value);
        virtual void tickTimer() = 0;
};
