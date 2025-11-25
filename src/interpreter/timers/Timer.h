#pragma once

#include <cstdint>
#include <chrono>
#include <ctime>
#include <thread>

class Timer {
    // Use join thread for safety
    std::jthread timerThread;
    bool isSoundTimer;

    protected:
        uint8_t m_timer {0};

    public:
        explicit Timer(bool isSoundTimer);
        void tickTimer();
        void setTimer(uint8_t value);
};
