#ifndef CHIP8_H
#define CHIP8_H
#include <iterator>
#include <algorithm>
#include <filesystem>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <map>
#include <random>
#include <time.h>

#define MEM_SIZE 4096
#define STACK_SIZE 64
#define NUM_VREGS 16
#define NUM_KEYS 16
#define DISP_X 64
#define DISP_Y 32
#define SPRITE_WIDTH 8

class Chip8;

// See section 2.4 - Display
// These sprites are 5 bytes long.
extern unsigned char textfont[80];

namespace Op {
	enum { CLS, RET, SYS, JP, CALL, 
		SE, SNE, LD, ADD, LDR, 
		OR, AND, XOR, SUB, SHR, 
		SUBN, SHL, RND, DRW, SKP,
		SKNP, ERR
	};

	const char* optostr(uint8_t op);

	// Chip8 is Big-endian
	// Big-endian reads from msb -> lsb
	// Examples: 3xkk, 5xy0
	// These extract the bits, not the values at the register
	// x = 4-bit index of the V register (V0-VF)
	size_t x(uint16_t opcode);
	// y = 4-bit index of the V register (V0-VF)
	size_t y(uint16_t opcode);
	// kk = 8-bit constant
	uint8_t kk(uint16_t opcode);
	// nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
	uint16_t nnn(size_t opcode);
	// n or nibble - A 4-bit value, the lowest 4 bits of the instruction
	uint8_t n(uint16_t opcode);

}



class Chip8 {
	public:
		Chip8(){
			srand(time(0)); // Init RNG
		}
		Chip8(const char* rom_path){
			srand(time(0)); // Init RNG
			LoadFont(textfont);
			LoadROM(rom_path);
		}
		uint8_t mem[MEM_SIZE] = {0}; // mem of the chip8
		uint8_t gfx[DISP_X * DISP_Y] = {0}; // 64x32 display
		uint8_t keys[NUM_KEYS] = {0};
		bool draw_flag = false; // draw flag

		bool LoadROM(const char* rom_path);
		// Load font set into memory
		void LoadFont(uint8_t* font);
		void print_display();
		void fill_gfx();
		void print_test();
		
};

#endif
