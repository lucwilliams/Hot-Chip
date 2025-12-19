#pragma once

#include <cstdint>
#include <span>

// A simple struct to hold read-only access to emulated memory.
// Used by the Window class to display debug info.
struct Chip8MemoryView {
    std::span<uint8_t> memory;
    std::span<uint8_t> registers;
    const uint16_t getPC;
    const uint16_t getIndexRegister;
};
