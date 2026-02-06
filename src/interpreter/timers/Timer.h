#pragma once

#include <thread>

class Timer {
    protected:
        std::uint8_t m_timer{0};

    public:
        void setTimer(std::uint8_t value) {
            m_timer = value;
        }

        // Require derived timer classes to provide methods to tick and reset the timer
        virtual void tickTimer() = 0;
        virtual void reset() = 0;

        virtual ~Timer() = default;
};
