#include <fstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "Chip8.h"

Chip8::Chip8(const std::string& fileName, const Window& window)
	: m_window{window}
{
	// Read ROM data
	std::ifstream inFS;

	// Add roms/ prefix to open from roms folder
	std::string filePath = "roms/" + fileName;

	// Open the file to read in binary
	// Use ifstream::ate to position the get pointer at the end of file
	inFS.open(filePath, std::ifstream::binary | std::ifstream::ate);

	if (inFS.is_open())
	{
		// Determine ROM size by checking the position of the get pointer
		const std::streampos size = inFS.tellg();

		// Check that ROM isn't greater than the space allocated to it in program memory
		// 4096 - 512 (offset of ROM in memory)
		if (size > (kMemorySize - kROMOffset))
			std::cerr << "ROM size exceeds maximum of 3584 bytes. " << filePath << std::endl;

		// Return get pointer to start of file
		inFS.seekg(0, std::ifstream::beg);

		// Read in ROM to emulated memory, offsetting by 512 bytes.
		// The initial 512 bytes of the Chip-8 memory was used to store
		// the interpreter code in the original hardware.
		inFS.read(reinterpret_cast<char*>(m_memory.begin() + kROMOffset), size);
	}
	else
	{
		std::cerr << "Error opening ROM: " << filePath << ", " << std::strerror(errno) << std::endl;
	}

	// Initialise font data. Start font data in position 0x50 (+80 bytes) as is conventional.
	std::copy(kFontData.begin(), kFontData.end(), m_memory.begin() + kFontOffset);
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

	while (running)
	{
		// Check all SDL events (acts more like a for loop on &event)
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
				break;
			}
		}

        m_window.Draw();
    }
		if (!running)
			break;

        // WIP
		// Execute 256 instructions in between checking the state of the SDL window again
		uint8_t i{0};
		while (i < 255)
		{
			// Our memory is 8 bits, but an instruction is 16 bits.
			// We convert two preceding bytes into one uint16_t
			const uint16_t instruction = (static_cast<uint16_t>(m_memory[m_PC]) << 8) | m_memory[m_PC + 1];
			++i;
		}

		m_window.Draw();
	}
}
