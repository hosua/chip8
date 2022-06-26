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

#define DEBUG_MODE false // Frame by frame execution
#define VERBOSE_CLOCK false
#define VERBOSE_CPU true
#define VERBOSE_DISPLAY false
#define VERBOSE_INPUT true

// 1/60 = 0.16666666 * 10^3 = 16667
#define TICK 16667

class Chip8;

// See section 2.4 - Display
// These sprites are 5 bytes long.
extern unsigned char textfont[80];
// Destroy SDL and exit
void ExitChip8();

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
		uint8_t mem[MEM_SIZE] = {0}; // mem of the chip8
		uint8_t gfx[DISP_X * DISP_Y] = {0}; // 64x32 display
		bool keys[NUM_KEYS] = {0}; // array of all keys from 0-F, 1 if pressed, 0 if unpressed
		bool draw_flag = false; // draw flag

		// Load ROM into memory
		bool LoadROM(const char* rom_path);

		// Constructors
		Chip8(){
			srand(time(0)); // Init RNG
			LoadFont(textfont);
		}

		Chip8(const char* rom_path){
			srand(time(0)); // Init RNG
			LoadFont(textfont);
			LoadROM(rom_path);
		}

	private:
		// Load font set into memory
		void LoadFont(uint8_t* font);
};

extern Chip8 chip8;

#endif
