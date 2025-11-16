#include "Chip8.h"

enum class opcode : uint8_t {
    CLEAR_DISPLAY = 0xE0,
    RETURN = 0xEE,
};

void Chip8::opcode0(uint16_t instruction) {
    const opcode lowByte = static_cast<opcode>(
        getLowByte(instruction)
    );

	switch (lowByte) {
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

// GOTO NNN
void Chip8::opcode1(uint16_t instruction) {
    // Update PC to new address from instruction
    m_PC = getAddressFromInstruction(instruction);
}

// VX = NN
void Chip8::opcode6(uint16_t instruction) {
    // Get register number
    uint8_t regNum = nibbleAt(instruction, 2);

    // Get 8-bit constant
    uint8_t newVal = getLowByte(instruction);

    // Set VX = NN
    m_registers[regNum] = newVal;
}

// VX += NN
void Chip8::opcode7(uint16_t instruction) {
    // Get register number
    uint8_t regNum = nibbleAt(instruction, 2);

    // Get 8-bit constant
    uint8_t newVal = getLowByte(instruction);

    // Add NN to VX
    m_registers[regNum] += newVal;
}

// I = NNN
void Chip8::opcodeA(uint16_t instruction) {
    // Get 12 byte address
    uint16_t address = getAddressFromInstruction(instruction);

    // Set I to NNN
    m_index = address;
}
