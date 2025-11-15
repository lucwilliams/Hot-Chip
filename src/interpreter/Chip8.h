#pragma once

#include <array>
#include <iostream>
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

    // Size of emulated memory (4096)
    static constexpr uint16_t kMemorySize = 0x1000;

    // Offset where ROM is loaded into emulated memory (512)
    static constexpr uint16_t kROMOffset = 0x200;

    // Offset where font data is loaded into emulated memory (80)
    static constexpr uint8_t kFontOffset = 0x50;

    // The value used with AND on a 16 bit instruction to obtain first nibble
    static constexpr uint16_t kFirstNibbleMask = 0xF000;

    // The value used with AND on a 16 bit instruction to obtain first byte
    static constexpr uint16_t kSecondByteMask = 0xFF;

    // Emulated memory
    std::array<uint8_t, kMemorySize> m_memory{};
    uint16_t m_ROMSize{};

    // Registers 0-9 + A-F (16 total)
    std::array<uint8_t, 16> m_registers{};

    // Index/Address register (12 bits wide)
    uint16_t m_index{};

    // Program counter
    // (start at first instruction of ROM)
    uint16_t m_PC{kROMOffset};

    // Window and m_debug are initialised in the constructor initialisation list
    Window m_window;
    bool m_debug;

    void decode(uint16_t instruction);

    // For opcodes with the 0**** prefix
    void opcode0(uint16_t instruction);

    public:
        Chip8(const std::string& fileName, const Window& window, bool debugEnabled);
        void start();
};
