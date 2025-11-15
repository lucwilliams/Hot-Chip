#include "Chip8.h"

enum class opcode : uint8_t {
    CLEAR_DISPLAY = 0xE0,
    RETURN = 0xEE,
};

void Chip8::opcode0(uint16_t instruction) {
    const opcode secondByte = static_cast<opcode>(
        instruction & kSecondByteMask
    );

	switch (secondByte) {
        case opcode::CLEAR_DISPLAY:
            if (m_debug)
                std::cout << "[DEBUG] clearDisplay() executed. (instruction 0x00E0)" << std::endl;

            m_window.clearDisplay();
            break;
        case opcode::RETURN:
            std::cerr << "[ERROR] 0xEE Instruction: RETURN not implemented yet." << std::endl;
            break;
    }
}