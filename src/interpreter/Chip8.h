#pragma once

#include <cstdint>
#include "../window/Window.h"

class Chip8 {
    uint8_t m_memory[4096] {};

    // Registers 0-9
    uint8_t V0 {};
    uint8_t V1 {};
    uint8_t V2 {};
    uint8_t V3 {};
    uint8_t V4 {};
    uint8_t V5 {};
    uint8_t V6 {};
    uint8_t V7 {};
    uint8_t V8 {};
    uint8_t V9 {};

    // Registers A-F
    uint8_t VA {};
    uint8_t VB {};
    uint8_t VC {};
    uint8_t VD {};
    uint8_t VE {};
    uint8_t VF {};

    // Index/Address register
    uint8_t I {};

    Window m_window {};

    public:
        explicit Chip8(char fileName[], const Window& window);
        void start();
};
