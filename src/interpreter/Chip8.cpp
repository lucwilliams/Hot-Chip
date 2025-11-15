#include <fstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include "Chip8.h"

Chip8::Chip8(const std::string& fileName, const Window& window, bool debugEnabled)
	: m_window{window}
	, m_debug{debugEnabled}
{
	// Read ROM data
	std::ifstream inFS;

	// Add roms/ prefix to open from roms folder
	std::string filePath = "roms/" + fileName;

	// Open the file to read in binary
	// Use ifstream::ate to position the get pointer at the end of file
	inFS.open(filePath, std::ifstream::binary | std::ifstream::ate);

	if (inFS.is_open()) {
		// Determine ROM size by checking the position of the get pointer
		m_ROMSize = static_cast<uint16_t>(inFS.tellg());

		// Check that ROM isn't greater than the space allocated to it in program memory
		// 4096 - 512 (offset of ROM in memory)
		if (m_ROMSize > (kMemorySize - kROMOffset))
			throw std::runtime_error("ROM size exceeds maximum of 3584 bytes: " + filePath);

		// Assume files must be even as instructions are always two bytes in size
		if (m_ROMSize % 2 != 0) {
			throw std::runtime_error("ROM is not of even size! File may be corrupted: " + filePath);
		}

		// Return get pointer to start of file
		inFS.seekg(0, std::ifstream::beg);

		// Read in ROM to emulated memory, offsetting by 512 bytes.
		// The initial 512 bytes of the Chip-8 memory was used to store
		// the interpreter code in the original hardware.
		inFS.read(reinterpret_cast<char*>(m_memory.begin() + kROMOffset), m_ROMSize);

		// Initialise font data. Start font data in position 0x50 (+80 bytes) as is conventional.
		std::copy(kFontData.begin(), kFontData.end(), m_memory.begin() + kFontOffset);
	} else {
		throw std::runtime_error("Error opening ROM: " + filePath + ", " + std::strerror(errno));
	}
}

void Chip8::decode(uint16_t instruction) {
	/*
	 * 00E0 (clear screen)
	 * 1NNN (jump)
	 * 6XNN (set register VX)
	 * 7XNN (add value to register VX)
	 * ANNN (set index register I)
	 * DXYN (display/draw)
	 */

	// Output instruction for debug
	if (m_debug)
		// Output in hexadecimal form with zero padding if necessary
		std::cout << "[DEBUG] Decoded Instruction: "
			<< std::setw(4) << std::setfill('0')
			<< std::hex << std::uppercase
			<< instruction << std::endl;

	const uint8_t firstNibble = static_cast<uint8_t>(
		(instruction & kFirstNibbleMask) >> 8
	);

	switch (firstNibble) {
		case 0:
			// CLEAR_DISPLAY and RETURN opcodes
			opcode0(instruction);
			break;
	}

	// Increment PC by 2 as instructions are two bytes in size
	m_PC += 2;
}

void Chip8::start() {
	SDL_Event event{};

	// Whether user has killed program
	bool running = true;

	// If all instructions have been executed in the ROM
	bool finished = false;

	// The memory location of the ROM's final valid instruction
	// Subtract two since instructions are two bytes in size.
	uint16_t finalInstruction{
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

			m_window.draw();
		}
	}
}
