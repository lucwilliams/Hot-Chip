#include <fstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include "Chip8.h"

Chip8::Chip8(const std::string& fileName, const Window& window)
	: m_window{window}
{
	// Read ROM data
	std::ifstream inFS;

	// Open the file to read in binary
	// Use ifstream::ate to position the get pointer at the end of file
	inFS.open(fileName, std::ifstream::binary | std::ifstream::ate);

	if (inFS.is_open()) {
		// Determine ROM size by checking the position of the get pointer
		m_ROMSize = static_cast<uint16_t>(inFS.tellg());

		// Check that ROM isn't greater than the space allocated to it in program memory
		// 4096 - 512 (offset of ROM in memory)
		if (m_ROMSize > (kMemorySize - kROMOffset))
			throw std::runtime_error("ROM size exceeds maximum of 3584 bytes: " + fileName);

		// Return get pointer to start of file
		inFS.seekg(0, std::ifstream::beg);

		// Read in ROM to emulated memory, offsetting by 512 bytes.
		// The initial 512 bytes of the Chip-8 memory was used to store
		// the interpreter code in the original hardware.
		inFS.read(reinterpret_cast<char*>(m_memory.begin() + kROMOffset), m_ROMSize);

		// Initialise font data. Start font data in position 0x50 (+80 bytes) as is conventional.
		std::copy(kFontData.begin(), kFontData.end(), m_memory.begin() + kFontOffset);
	} else {
		throw std::runtime_error("Error opening ROM: " + fileName + ", " + std::strerror(errno));
	}
}

void Chip8::decode(uint16_t instruction) {
	// Output instruction for debug
	if (kDebugEnabled)
		// Output in hexadecimal form with zero padding if necessary
		std::cout << "[DEBUG] Decoded Instruction: "
			<< std::setw(4) << std::setfill('0')
			<< std::hex << std::uppercase
			<< instruction << std::endl;

	// Fourth nibble is the most significant
	const uint8_t highestNibble = nibbleAt(instruction, 3);

    // Keep track of whether PC has updated
    m_PCUpdated = false;

	// Map instruction to opcode function call
	switch (highestNibble) {
		case 0x0:
			// CLEAR_DISPLAY and RETURN instructions
			opcode0(instruction);
			break;
        case 0x1:
            // Jump to NNN
            opcode1(instruction);
            break;
        case 0x2:
            // Call subroutine at NNN
            opcode2(instruction);
            break;
        case 0x3:
            // Skip next instruction if (Vx == NN)
            opcode3(instruction);
            break;
		case 0x4:
			// Skip next instruction if (Vx != NN)
			opcode4(instruction);
			break;
		case 0x5:
			// Skip next instruction if (Vx == Vy)
			opcode5(instruction);
			break;
		case 0x6:
			// Set register VX to NN
			opcode6(instruction);
			break;
		case 0x7:
			// Add NN to VX
			opcode7(instruction);
			break;
        case 0x8:
            // Math and bitwise operations on registers
            opcode8(instruction);
            break;
		case 0x9:
			// Skip next instruction if (Vx != Vy)
			opcode9(instruction);
			break;
		case 0xA:
			// Set I to NNN
			opcodeA(instruction);
			break;
		case 0xB:
			// Jump to NNN plus the value in V0
			opcodeB(instruction);
			break;
		case 0xC:
			// Generate a random number in range of 0 -> NN (max 255)
			opcodeC(instruction);
			break;
		case 0xD:
			// Draw sprite at (VX, VY) with height N
			opcodeD(instruction);
			break;
        case 0xF:
            // Timers, keystrokes and misc memory instructions
            opcodeF(instruction);
            break;
		default:
			if (kDebugEnabled)
				std::cout << "[DEBUG] Opcode not implemented: " << instruction << std::endl;
	}

	// Do not increment PC for jump or return instructions
    if (!m_PCUpdated)
		// Increment PC by 2 as instructions are two bytes in size
		m_PC += 2;
}

void Chip8::start() {
	// The memory location of the ROM's final valid instruction
	// Subtract two since instructions are two bytes in size.
	const uint16_t finalInstruction {
		static_cast<uint16_t>(kROMOffset + m_ROMSize - 2)
	};

	// Fetch/decode/execute loop
	while (running) {
		// Check all SDL events (acts more like a for loop on &event)
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
				break;
			}
		}

		if (!running)
			break;

		if (finished) {
			// Sleep until the user closes the window
			SDL_WaitEvent(&event);
		} else {
			// Execute 256 instructions each frame.
			uint8_t instructionsExecuted{0};
			while (instructionsExecuted < 255) {
				if (m_PC > finalInstruction) {
					// All instructions have completed
					finished = true;
					break;
				} else if (m_PC + 1 < kMemorySize)  {
					// Fetch instruction:
					// Our memory is 8 bits, but an instruction is 16 bits.
					// We concatenate the byte at PC with the byte that follows to form one uint16_t
					uint16_t instruction = static_cast<uint16_t>(m_memory[m_PC]) << 8 | m_memory[m_PC + 1];
					decode(instruction);
					++instructionsExecuted;
				} else {
					throw std::runtime_error(
						"Cannot run out-of-bounds instruction at position: "
						+ std::to_string(m_PC) + "."
					);
				}
			}

            m_window.render();
		}
	}
}
