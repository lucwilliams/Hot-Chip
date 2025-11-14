#pragma once

#include <array>
#include "../window/Window.h"

class Chip8 {
    // Character representations for 0-9 + A-F
    // https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#font
    static constexpr std::array<uint8_t, 80> kFontData = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // Size of emulated memory
    static constexpr uint16_t kMemorySize = 4096;

    // Offset where ROM is loaded into emulated memory
    static constexpr uint16_t kROMOffset = 512;

    // Offset where font data is loaded into emulated memory
    static constexpr uint8_t kFontOffset = 80;

    // Emulated memory
    std::array<uint8_t, kMemorySize> m_memory{};

    // Registers 0-9 + A-F (16 total)
    std::array<uint8_t, 16> m_registers{};

    // Index/Address register (12 bits wide)
    uint16_t m_index{};

    // Program counter
    // (start at first instruction of ROM)
    uint16_t m_PC{kROMOffset};

    // Initialise window in constructor initialisation list
    Window m_window;

    public:
        Chip8(char fileName[], const Window& window);
        Chip8(const std::string& fileName, const Window& window);
        void start();
};
