#ifndef CPU_H
#define CPU_H

#include <chip8.h>

class CPU {
	public:
		Chip8* chip8;
		CPU(Chip8* chip8){
			this->chip8 = chip8;
			this->mem = chip8->mem;
		}

		uint8_t stack[STACK_SIZE] = {0}; // The 64-byte stack
		uint8_t sp = 0x0; // 8-bit Stack pointer
		uint8_t v[NUM_VREGS] = {0}; // Vx registers
		uint16_t i = 0x0; // 16-bit index register. Stores memory addresses
		uint16_t pc = 0x200; // Program counter (set it to the beginning of ROM)
		uint8_t dt = 0x0; // Delay timer
		uint8_t st = 0x0; // 8-bit Sound timer
		uint8_t *mem;
		uint16_t opcode = 0;


		// Fetches 2-byte (16-bit) instructions
		void cycle();
		uint8_t decode(uint16_t opcode);

		// Execute CPU instructions
		void execute(uint8_t op);
			
		/* debugging functions */
		void print_registers();
		void print_stack();
		void print_args(uint16_t opcode, size_t x, size_t y, uint8_t kk, uint16_t nnn, uint8_t n);
		
};

#endif 
