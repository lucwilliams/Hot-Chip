#include "Timer.h"

// Initialise thread using tickTimer function
Timer::Timer(bool isSoundTimer)
    : timerThread {&Timer::tickTimer, this}
    , isSoundTimer {isSoundTimer}
{}

void Timer::tickTimer() {
    // Decrement timer 60 times a second (60Hz)
    std::this_thread::sleep_for(
        std::chrono::duration<double>(1.0/60.0)
    );
    --m_timer;

    if (isSoundTimer && (m_timer > 0)) {
        // beep!
    }
}

void Timer::setTimer(uint8_t value) {
    m_timer = value;
}