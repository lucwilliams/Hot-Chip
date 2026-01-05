#include <chrono>
#include "DelayTimer.h"

void DelayTimer::tickTimer(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        // Decrement timer 60 times a second (60Hz)
        std::this_thread::sleep_for(
            std::chrono::duration<double>(1.0/60.0)
        );

        if (m_timer > 0)
            --m_timer;
    }
}

void DelayTimer::reset() {
    // End timer thread
    m_timerThread.request_stop();
    m_timerThread.join();

    // Reset count
    m_timer = 0;

    // Create new thread
    m_timerThread = std::jthread(&DelayTimer::tickTimer, this);
}

uint8_t DelayTimer::readTimer() {
    return m_timer;
}

DelayTimer::DelayTimer()
{
    m_timerThread = std::jthread(&DelayTimer::tickTimer, this);
}

DelayTimer::~DelayTimer() {
    m_timerThread.request_stop();
}