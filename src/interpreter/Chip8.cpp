#include "Chip8.h"
#include <fstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

namespace {
    // https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#font
    constexpr uint8_t kFontData[] = {
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
}

Chip8::Chip8(char fileName[], const Window& window)
    : m_window {window}
{
    // Initialise emulator by reading ROM
    std::ifstream inFS;

    // Add roms/ prefix to open from roms folder
    char filePath[] = "roms/";
    strcat(filePath, fileName);

    // Open the file to read in binary
    // Use ifstream::ate to position the get pointer at the end of file
    inFS.open(filePath, std::ifstream::binary | std::ifstream::ate);

    if (inFS.is_open()) {
        // Determine ROM size by checking the position of the get pointer
        const std::streampos size = inFS.tellg();

        // Return get pointer to start of file
        inFS.seekg(0, std::ifstream::beg);

        // Read in ROM, offsetting by 512 bytes (position 0x200)
        // The initial 512 bytes of the Chip-8 memory was used to store the interpreter code in the original hardware.
        inFS.read(reinterpret_cast<char*>(m_memory + 0x200), size);
    } else {
        std::cerr << "Error opening ROM: " << filePath << ", " << std::strerror(errno) << std::endl;
    }

    // Initialise font data. Start font data in position 0x50 (+80 bytes) as is conventional.
    std::move(kFontData, kFontData, m_memory + 0x50);
}

void Chip8::start() {
    bool running {true};
    SDL_Event event;

    // Window demo (smiley face)

    // Eyes
    m_window.flipPixel(25, 10);
    m_window.flipPixel(35, 10);

    // Grin
    m_window.flipPixel(23, 13);
    m_window.flipPixel(37, 13);

    // Smile
    for (int i {24}; i < 37; ++i)
        m_window.flipPixel(i, 14);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
        }

        m_window.Draw();
    }
}
