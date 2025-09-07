#include "chip8.h"
#include <fstream>
#include <cstring>
#include <iostream>
#include <random>

// CHIP-8 fontset, each character is 5 bytes
const uint8_t chip8_fontset[80] = {
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


Chip8::Chip8() {
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;


	// Clear display
	std::memset(gfx, 0, sizeof(gfx));
	// Clear stack, registers, memory
	std::memset(stack, 0, sizeof(stack));
	std::memset(V, 0, sizeof(V));
	std::memset(memory, 0, sizeof(memory));
	std::memset(keypad, 0, sizeof(keypad));

	// Reset timers
	delayTimer = 0;
	soundTimer = 0;

	drawFlag = false;

	// Load fontset into memory
	for (int i = 0; i< 80; ++i) {
		memory[0x50 + i] = chip8_fontset[i];
	}

	gen = std::mt19937(std::random_device{}());
	dist = std::uniform_int_distribution<uint8_t>(0, 255);


}

void Chip8::LoadROM(const char* filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open()) {
		std::cerr << "Failed to open ROM: " << filename << std::endl;
		return;
	}

	std::streampos size = file.tellg();
	char* buffer = new char[size];

	file.seekg(0, std::ios::beg);
	file.read(buffer, size);
	file.close();

	// Load ROM bytes into memory at 0x200
	for (int i = 0; i < size; ++i) {
		memory[0x200 + i] = buffer[i];
	}

	delete[] buffer;
}

void Chip8::Cycle() {
	//Fetch two bytes from memory where the pc is pointing
	opcode = memory[pc] << 8 | memory[pc + 1];    // Combines two bytes into a halfword

	// Increment pc
	pc += 2;

	// Decode and Execute
	ExecuteOpcode();  // TO BE IMPLEMENTED!!!
	

	// Update timers
	if (delayTimer > 0) --delayTimer;

	if (soundTimer > 0) {
		if (soundTimer == 1) {
			// Sound cue
		}
		--soundTimer;
	}
}

void Chip8::ExecuteOpcode() {
        switch(opcode & 0xF000) {
                case 0x0000:
                        switch(opcode & 0x00FF) {
                                case 0x00E0:
					std::memset(gfx,0, sizeof(gfx));
					drawFlag = true;
					break;
                                case 0x00EE:
					if (sp > 0) {
                                        	--sp;
					}
					pc = stack[sp];
					break;
                                default:
					std::cout << "Unrecognized command: " << opcode << std::endl; 
                        }
                        break;
                case 0x1000:
			pc = opcode & 0x0FFF;
                        break;
                case 0x2000:
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
                        break;
                case 0x3000:
			uint8_t Vx = V[(opcode & 0x0F00) >> 8];
			uint8_t byte = opcode & 0x00FF;
			if (Vx == byte) pc+=2;
                        break;
                case 0x4000:
			{
				uint8_t Vx = V[(opcode & 0x0F00) >> 8];
				uint8_t byte = opcode & 0x00FF;
				if (Vx != byte) pc+=2;
			}
			break;
                case 0x5000:
			if (opcode & 0x000F == 0) {
				uint8_t Vx = V[(opcode & 0x0F00) >> 8];
				uint8_t Vy = V[(opcode & 0x00F0) >> 4];
				if (Vx == Vy) pc += 2;
			}
                        break;
                case 0x6000:
			{
				uint8_t byte = opcode & 0x00FF;
				V[(opcode & 0x0F00) >>8] = byte;
			}
			break;
                case 0x7000:
			{
				uint8_t byte = opcode & 0x00FF;
				V[(opcode & 0x0F00) >> 8] += byte;
			}
			break;
                case 0x8000:
                        switch(opcode & 0x000F) {
                                case 0x0000:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                                        break;
                                case 0x0001:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4]; 
                                        break;
                                case 0x0002:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                                        break;
                                case 0x0003:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
                                        break;
                                case 0x0004:
					{
                                                uint16_t sum = V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4];
                                                V[15] = (sum > 255) > 1 : 0;
                                                V[(opcode & 0x0F00) >> 8] = sum & 0x00FF;
                                        }
                                        break;
                                case 0x0005:
					{
						V[15] = (V[(opcode & 0x0F00)>>8] > V[(opcode & 0x00F0) >> 4]) > 1 : 0;
						V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					}
                                        break;
                                case 0x0006:
					uint8_t lsb = V[(opcode & 0x0F00) >> 8] & 0x0001;
					V[(opcode & 0x0F00) >> 8] >>=1;
					V[15] = lsb;
                                        break;
                                case 0x0007:
					{
                                                V[15] = (V[(opcode & 0x00F0)>>4] > V[(opcode & 0x0F00) >> 8]) > 1 : 0;
                                                V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00)>> 8];
                                        }
                                        break;
                                case 0x000E:
					uint8_t msb = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7;
                                        V[(opcode & 0x0F00) >> 8] <<=1;
                                        V[15] = msb;
                                        break;
                        break;
                        }
                case 0x9000:
			if ((opcode & 0x000F) == 0) {
				uint8_t Vx = V[(opcode & 0x0F00) >> 8];
                        	uint8_t Vy = V[(opcode & 0x00F0) >> 4];
                        	if (Vx != Vy) pc += 2;
			}
                        break;
                case 0xA000:
			{
				I = opcode & 0x0FFF;
			}
                        break;
                case 0xB000:
			{
				PC = V[0] + (opcode & 0x0FFF);
			}
                        break;
                case 0xC000:
			V[(opcode & 0x0F00)>>8] = dist(gen) & (opcode & 0x00FF);
                        break;
		case 0xD001:
			{
				uint8_t X = V[(opcode & 0x0F00) >> 8];
				uint8_t Y = V[(opcode & 0x00F0) >> 4];
				uint8_t N = (opcode & 0x000F);
				V[0xF] = 0;

				for (int j = 0; j < N; j++) {
					uint8_t spriteByte = memory[I+j];

					for (int i = 0; i < 8; i++) {
						uint8_t spritePixel = (spriteByte >> (7 - i)) & 0x1;
						uint16_t xCoord = (X + i) % 64;
						uint16_t yCoord = (Y + j) % 32;
						uint16_t index = xCoord + (yCoord * 64);

						if (spritePixel) {
							if (gfx[index] == 1) {
								V[0xF] = 1;
							}	
							gfx[index] ^= 1;
						}
					}
				}
			
				drawFlag = true;
				break;
			}
			
                        
                case 0xE000:
                        if (opcode & 0x00FF == 0x009E) {
                                break;
                        } else if (opcode & 0x00FF == 0x00A1) {
                                break;
                        }
                        break;
                case 0xF000:
                        if (opcode & 0x00FF == 0x0007) {
                                break;
                        } else if (opcode & 0x00FF == 0x000A) {
                                break;
                        } else if (opcode & 0x00FF == 0x0015) {
                                break;
                        } else if (opcode & 0x00FF == 0x0018) {
                                break;
                        } else if (opcode & 0x00FF == 0x001E) {
                                break;
                        } else if (opcode & 0x00FF == 0x0029) {
                                break;
                        } else if (opcode & 0x00FF == 0x0033) {
                                break;
                        } else if (opcode & 0x00FF == 0x0055) {
                                break;
                        } else if (opcode & 0x00FF == 0x0065) {
                                break;
                        }
                        break;
                default:
                        std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;

        }
}

