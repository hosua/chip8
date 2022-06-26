#ifndef CPU_H
#define CPU_H

#include <chip8.h>
#include <clock.h>
#include <stack>

class CPU {
	public:
		std::stack<uint16_t> stack;
		Chip8* chip8;
		Clock* clock;
		uint8_t v[NUM_VREGS] = {0}; // Vx registers
		uint16_t i = 0x0; // 16-bit index register. Stores memory addresses
		uint16_t pc = 0x200; // Program counter (set it to the beginning of ROM)
		uint8_t dt = 0x0; // Delay timer
		uint8_t st = 0x0; // 8-bit Sound timer
		uint8_t *mem; // Points to the chip8's mem
		uint16_t opcode = 0;

		// Constructors
		CPU(Chip8* chip8) : chip8(chip8), mem(chip8->mem) {}
		CPU(Chip8* chip8, Clock* clock) : chip8(chip8), clock(clock), mem(chip8->mem) {}

		// Fetches 2-byte (16-bit) instructions
		void cycle();
		// Counts down dt when it is non-zero
		void delay_timer();

		// Decode an opcode for so the CPU can understand it
		uint8_t decode(uint16_t opcode);

		// Execute CPU instruction
		void execute(uint8_t op);
			
		/* debugging functions */
		void print_registers();
		void print_args(uint16_t opcode);
		
};

#endif 
