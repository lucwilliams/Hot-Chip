#pragma once

#include <cstdint>
#include <span>
#include "../utils/RingBuffer.h"

// A simple struct to hold read-only access to emulated memory.
// Used by the Window class to display debug info.
struct Chip8DebugData {
    std::span<std::uint8_t> memory;
    std::span<std::uint8_t> registers;
    const std::uint16_t PC;
    const std::uint16_t index;
};
