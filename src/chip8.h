#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <array>

class Chip8 {
	public:
		Chip();
		void Cycle();        // Once cycle of fetch-execute cycle
		void LoadROM(const char* filename);
		uint8_t gfc[64*32];  // 64*32 monochrome display
		uint8_t delayTimer; 
		uint8_t soundTimer;
		uint8_t keypad[16];

		bool drawFlag;

		std::mt19937 gen;
		std::uniform_int_distribution<uint8_t> dist;

	private:
		uint16_t opcode;          // Current Opcode
		uint8_t memory[4096];    // 4096 bytes memory
		uint8_t V[16];           // Registers V0 - VF
		uint16_t I;              // Index Register
		uint16_t pc;             // Program Counter
		uint16_t stack[16];      // Stack
		uint16_t sp;             // Stack Pointer

		void ExecuteOpcode();
}

#endif
