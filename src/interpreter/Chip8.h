#pragma once

#include <array>
#include <random>
#include <bitset>
#include "../window/MainWindow.h"
#include "timers/SoundTimer.h"
#include "timers/DelayTimer.h"
#include "SafeArray.h"

// Compile with -DDEBUG for debug output
#ifdef DEBUG
    inline constexpr bool kDebugEnabled = true;
#else
    inline constexpr bool kDebugEnabled = false;
#endif

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

    // Offset where ROM is loaded into emulated memory (512).
    // Although they're called ROMs, programs can actually
    // write to their data in memory.
    static constexpr uint16_t kROMOffset = 0x200;

    // Offset where font data is loaded into emulated memory (80)
    static constexpr uint8_t kFontOffset = 0x50;

    // The bit length of a nibble
    static constexpr uint8_t kNibbleLength = 4;

    // Amount of emulated registers
    static constexpr uint8_t kRegisterAmount = 16;

    // The value used with AND on a 16 bit instruction to obtain a nibble
    // 00001111 in binary
    static constexpr uint8_t kNibbleMask = 0xF;

    // The value used with AND on a 16 bit instruction to obtain the least significant byte
    // 11111111 in binary
    static constexpr uint8_t kLowerByteMask = 0xFF;

    // The value used with AND on a 16 bit instruction
    // to obtain the lower 12 bits for addressing
    // 111111111111 in binary
    static constexpr uint16_t kAddressMask = 0xFFF;

    // The value used with AND to obtain the MSB of a byte
    // 10000000 in binary
    static constexpr uint16_t kMSBMask = 0x80;

    /*
     * m_ROMPath contains the actual file path of the currently loaded ROM.
     * m_sharedROMPath contains the desired path chosen by the UI.
     *
     * Once m_sharedROMPath updates, the emulator loads the ROM at that path,
     * and m_ROMPath updates to use the path stored in m_sharedROMPath.
     */
    std::string m_ROMPath;

    // TODO: Consider alternative container for m_sharedROMPath
    std::shared_ptr<std::string> m_sharedROMPath{std::make_shared<std::string>()};

    // Emulated memory
    SafeArray<kMemorySize, kDebugEnabled> m_memory{};
    uint16_t m_ROMSize{};

    // Registers 0-9 + A-F (16 total)
    SafeArray<kRegisterAmount, kDebugEnabled> m_registers{};

    // Index/Address register (12 bits wide)
    uint16_t m_index{};

    // Program counter
    // (start at first instruction of ROM)
    uint16_t m_PC{kROMOffset};

    // Stack, just for subroutine return addresses
    // TODO: Use SafeArray for m_stack?
    std::array<uint16_t, 16> m_stack{};
    uint8_t m_stackSize {0};

    // Internal bool to determine whether the PC is to be incremented.
    // (don't increment the PC following jump or return instructions)
    bool m_PCUpdated = false;

    // Mersenne Twister algorithm used for RNG
    std::mt19937 m_mersenneTwister{
        static_cast<std::mt19937::result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        )
    };

    // Generate a random value from 0 to 255 (max value of uint8_t)
    std::uniform_int_distribution<uint8_t> m_randUint8{0, 255};

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

    // Second step of the fetch/decode/execute loop
    void decode(uint16_t instruction);

    // -- Helper functions for manipulating opcodes --

    // Helper function to obtain a 4 bit nibble by position from a 16 bit opcode
    static uint8_t nibbleAt(uint16_t instruction, uint8_t position) {
        // Position is an index starting at zero and
        // must not exceed the index of the fourth nibble.
        if (position > kNibbleLength - 1)
            throw std::runtime_error(
                "[ERROR] Out of range byte access in nibbleAt() call with position: "
                + std::to_string(position)
            );

        // Byte mask of 8 bits at desired position.
        uint16_t positionMask = kNibbleMask << position * kNibbleLength;

        const auto byte = static_cast<uint8_t>(
            (instruction & positionMask) >> position * kNibbleLength
        );

        return byte;
    }

    // Helper function to obtain a 12 bit address (NNN) from a 16 bit opcode
    static uint8_t getLowByte(uint16_t instruction) {
        const auto lowByte = static_cast<uint8_t>(
            instruction & kLowerByteMask
        );
        return lowByte;
    }

    // Helper function to obtain a 12 bit address (NNN) from a 16 bit opcode
    static uint16_t getAddressFromInstruction(uint16_t instruction) {
        const uint16_t address = instruction & kAddressMask;
        return address;
    }

    static uint8_t scanCodeToPos(SDL_Scancode scanCode) {
        uint8_t pos{0};

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
        uint8_t pos = scanCodeToPos(scanCode);
        m_keyStates[pos] = state;
    }

    // All emulated instruction opcodes by prefix
    void opcode0(uint16_t instruction);
    void opcode1(uint16_t instruction);
    void opcode2(uint16_t instruction);
    void opcode3(uint16_t instruction);
    void opcode4(uint16_t instruction);
    void opcode5(uint16_t instruction);
    void opcode6(uint16_t instruction);
    void opcode7(uint16_t instruction);
    void opcode8(uint16_t instruction);
    void opcode9(uint16_t instruction);
    void opcodeA(uint16_t instruction);
    void opcodeB(uint16_t instruction);
    void opcodeC(uint16_t instruction);
    void opcodeD(uint16_t instruction);
    void opcodeE(uint16_t instruction);
    void opcodeF(uint16_t instruction);

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
        Chip8(const std::string& fileName, MainWindow& window);
        void start();
};
