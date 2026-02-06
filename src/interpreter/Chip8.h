#pragma once

#include <array>
#include <random>
#include <bitset>
#include "timers/SoundTimer.h"
#include "timers/DelayTimer.h"
#include "../utils/SafeArray.h"
#include "../utils/RingBuffer.h"
#include "../window/MainWindow.h"

// Compile with -DDEBUG for debug output
#ifdef DEBUG
    inline constexpr bool kDebugEnabled = true;
#else
    inline constexpr bool kDebugEnabled = false;
#endif

// For Windows platform-specific timing
#if defined(_WIN64)
	#include <Windows.h>
	#include <timeapi.h>
#endif

class Chip8 {
    // Character representations for 0-9 + A-F
    // https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#font
    static constexpr std::array<std::uint8_t, 80> kFontData = {
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
    static constexpr std::uint16_t kMemorySize = 0x1000;

    // Offset where ROM is loaded into emulated memory (512).
    // Although they're called ROMs, programs can actually
    // write to their data in memory.
    static constexpr std::uint16_t kROMOffset = 0x200;

    // Offset where font data is loaded into emulated memory (80)
    static constexpr std::uint8_t kFontOffset = 0x50;

    // The bit length of a nibble
    static constexpr std::uint8_t kNibbleLength = 4;

    // Amount of emulated registers
    static constexpr std::uint8_t kRegisterAmount = 16;

    // The value used with AND on a 16 bit instruction to obtain a nibble
    // 00001111 in binary
    static constexpr std::uint8_t kNibbleMask = 0xF;

    // The value used with AND on a 16 bit instruction to obtain the least significant byte
    // 11111111 in binary
    static constexpr std::uint8_t kLowerByteMask = 0xFF;

    // The value used with AND on a 16 bit instruction
    // to obtain the lower 12 bits for addressing
    // 111111111111 in binary
    static constexpr std::uint16_t kAddressMask = 0xFFF;

    // The value used with AND to obtain the MSB of a byte
    // 10000000 in binary
    static constexpr std::uint16_t kMSBMask = 0x80;

    // Use constant instructions per frame (IPF) for now
    static constexpr std::uint16_t kInstructionsPerFrame = 22;

    // Assume 60fps constant frame timing for now.
    static constexpr auto kFrameDuration = std::chrono::duration<double>(1.0 / 60.0);

    // Handle to waitable timer object for Windows
    #if defined(_WIN64)
        HANDLE m_winTimerHandle = nullptr;
    #endif

    // m_ROMPath contains the file path of the currently loaded ROM.
    std::string m_ROMPath;

    // Emulated memory
    SafeArray<kMemorySize, kDebugEnabled> m_memory{};
    std::uint16_t m_ROMSize{};

    // Registers 0-9 + A-F (16 total)
    SafeArray<kRegisterAmount, kDebugEnabled> m_registers{};

    // Index/Address register (12 bits wide)
    std::uint16_t m_index{};

    // Program counter
    // (start at first instruction of ROM)
    std::uint16_t m_PC{kROMOffset};

    // Stack, just for subroutine return addresses
    // TODO: Use SafeArray for m_stack?
    std::array<std::uint16_t, 16> m_stack{};
    std::uint8_t m_stackSize {0};

    // Internal bool to determine whether the PC is to be incremented.
    // (don't increment the PC following jump or return instructions)
    bool m_PCUpdated = false;

    // Mersenne Twister algorithm used for RNG
    std::mt19937 m_mersenneTwister{
        static_cast<std::mt19937::result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        )
    };

    // Generate a random value from 0 to 255 (max value of std::uint8_t)
    std::uniform_int_distribution<std::uint8_t> m_randUint8{0, 255};

    // Window is initialised in the constructor initialisation list
    MainWindow& m_window;

    SoundTimer m_soundTimer;
    DelayTimer m_delayTimer;

    // Whether the user has closed the window
    bool m_windowClosed = false;

    // Boolean array representing pressed state of all 16 keypad inputs
    std::bitset<16> m_keyStates{};

    // For handling user input events
    SDL_Event m_event{};

    // Boolean used to block execution on AWAIT_KEY instruction
    bool m_awaitingKey = false;

    // Use SDL_SCANCODE_UNKNOWN when the key is yet to be pressed.
    static constexpr SDL_Scancode kNULLScanCode = SDL_SCANCODE_UNKNOWN;

    // The key pressed by the AWAIT_KEY instruction, to await its release.
    SDL_Scancode m_awaitingScanCode = kNULLScanCode;

    // The register number used to store keypress of AWAIT_KEY instruction
    std::uint8_t m_awaitingKeyRegNum{0};

    // Second step of the fetch/decode/execute loop
    void decode(std::uint16_t instruction);

    // -- Helper functions for manipulating opcodes --

    // Helper function to obtain a 4 bit nibble by position from a 16 bit opcode
    static std::uint8_t nibbleAt(std::uint16_t instruction, std::uint8_t position) {
        // Position is an index starting at zero and
        // must not exceed the index of the fourth nibble.
        if (position > kNibbleLength - 1)
            throw std::runtime_error(
                "[ERROR] Out of range byte access in nibbleAt() call with position: "
                + std::to_string(position)
            );

        // Byte mask of 8 bits at desired position.
        std::uint16_t positionMask = kNibbleMask << position * kNibbleLength;

        const auto byte = static_cast<std::uint8_t>(
            (instruction & positionMask) >> position * kNibbleLength
        );

        return byte;
    }

    // Helper function to obtain a 12 bit address (NNN) from a 16 bit opcode
    static std::uint8_t getLowByte(std::uint16_t instruction) {
        const auto lowByte = static_cast<std::uint8_t>(
            instruction & kLowerByteMask
        );
        return lowByte;
    }

    // Helper function to obtain a 12 bit address (NNN) from a 16 bit opcode
    static std::uint16_t getAddressFromInstruction(std::uint16_t instruction) {
        const std::uint16_t address = instruction & kAddressMask;
        return address;
    }

    static std::uint8_t scanCodeToPos(SDL_Scancode scanCode) {
        std::uint8_t pos{0};

        /*
         * All 16 buttons used for the Chip-8 keypad.
         * Order of enum maps to array index.
         *
         * Scancodes are used over key names for
         * consistency across keyboards layouts.
         */
        switch(scanCode) {
            case SDL_SCANCODE_1: pos = 0x1; break;
            case SDL_SCANCODE_2: pos = 0x2; break;
            case SDL_SCANCODE_3: pos = 0x3; break;
            case SDL_SCANCODE_4: pos = 0xC; break;
            case SDL_SCANCODE_Q: pos = 0x4; break;
            case SDL_SCANCODE_W: pos = 0x5; break;
            case SDL_SCANCODE_E: pos = 0x6; break;
            case SDL_SCANCODE_R: pos = 0xD; break;
            case SDL_SCANCODE_A: pos = 0x7; break;
            case SDL_SCANCODE_S: pos = 0x8; break;
            case SDL_SCANCODE_D: pos = 0x9; break;
            case SDL_SCANCODE_F: pos = 0xE; break;
            case SDL_SCANCODE_Z: pos = 0xA; break;
            case SDL_SCANCODE_X: pos = 0x0; break;
            case SDL_SCANCODE_C: pos = 0xB; break;
            case SDL_SCANCODE_V: pos = 0xF; break;

            // Unused key
            default: break;
        }

        return pos;
    }

    void setKeyState(SDL_Scancode scanCode, bool state) {
        // Position in m_keyStates array
        std::uint8_t pos = scanCodeToPos(scanCode);
        m_keyStates[pos] = state;
    }

    // All emulated instruction opcodes by prefix
    void opcode0(std::uint16_t instruction);
    void opcode1(std::uint16_t instruction);
    void opcode2(std::uint16_t instruction);
    void opcode3(std::uint16_t instruction);
    void opcode4(std::uint16_t instruction);
    void opcode5(std::uint16_t instruction);
    void opcode6(std::uint16_t instruction);
    void opcode7(std::uint16_t instruction);
    void opcode8(std::uint16_t instruction);
    void opcode9(std::uint16_t instruction);
    void opcodeA(std::uint16_t instruction);
    void opcodeB(std::uint16_t instruction);
    void opcodeC(std::uint16_t instruction);
    void opcodeD(std::uint16_t instruction);
    void opcodeE(std::uint16_t instruction);
    void opcodeF(std::uint16_t instruction);

    /*
     * Private member functions used to initialise and
     * reset the state of the emulator.
     */
    void loadROM();
    void resetEmulator();

    /*
     * Main execution loop of the emulator.
     *
     * The state of the window (closed or running)
     * determines whether the emulation is still running.
     */
    void executionLoop();

    public:
        Chip8(MainWindow& window);
        ~Chip8();
        void start();
};
