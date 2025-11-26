#include "Timer.h"

// Initialise thread using tickTimer function
Timer::Timer()
    : timerThread {&Timer::tickTimer, this}
{}

void Timer::setTimer(uint8_t value) {
    m_timer = value;
}