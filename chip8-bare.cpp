// Sources used
// https://blog.wjdevschool.com/blog/video-game-console-emulator/
// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
// http://www.codeslinger.co.uk/pages/projects/chip8/fetchdecode.html

#include <iostream>
#include <iterator>
#include <algorithm>
#include <filesystem>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <map>
#include <random>
#include <time.h>

// MEM_SIZE is how much mem the chip8 can access
#define MEM_SIZE 4096
#define STACK_SIZE 64
#define NUM_VREGS 16
#define NUM_KEYS 16
#define DISP_X 64
#define DISP_Y 32

using std::cout;
using std::endl;
using std::string;

class Chip8;

void print_args(uint16_t opcode, size_t x, size_t y, uint8_t kk, uint16_t nnn, uint8_t n);


// See section 2.4 - Display
// "These sprites are 5 bytes long"
unsigned char textfont[80] = {
	// 1, 2 	3, 	  4, 	5 bytes
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
	0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

namespace Op {
	enum { CLS, RET, SYS, JP, CALL, 
			SE, SNE, LD, ADD, LDR, 
			OR, AND, XOR, SUB, SHR, 
			SUBN, SHL, RND, DRW, SKP,
			SKNP,
	};

	const char* optostr(uint8_t op){
		switch(op){
			case CLS: return "CLS";
			case RET: return "RET";
			case SYS: return "SYS";
			case JP: return "JP";
			case CALL: return "CALL";
			case SE: return "SE";
			case SNE: return "SNE";
			case LD: return "LD";
			case ADD: return "ADD";
			case LDR: return "LDR";
			case OR: return "OR";
			case AND: return "AND";
			case XOR: return "XOR";
			case SUB: return "SUB";
			case SHR: return "SHR";
			case SUBN: return "SUBN";
			case SHL: return "SHL";
			case RND: return "RND";
			case DRW: return "DRW";
			case SKP: return "SKP";
			case SKNP: return "SKNP";
			default: return "WTF???";
		}
		return "";
	}
	// Chip8 uses Big-endian
	// Big-endian reads from msb -> lsb
	// Examples: 3xkk, 5xy0
	// x = 4-bit index of the V register (V0-VF)
	size_t x(uint16_t opcode){
		return (opcode & 0x0F00) >> 8; 
	}
	// y = 4-bit index of the V register (V0-VF)
	size_t y(uint16_t opcode){
		return (opcode & 0x00F0) >> 4;
	}
	// kk = 8-bit constant
	uint8_t kk(uint16_t opcode){
		return (opcode & 0x00FF);
	}
	// nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
	uint16_t nnn(size_t opcode){
		return (opcode & 0x0FFF);
	}
	// n or nibble - A 4-bit value, the lowest 4 bits of the instruction
	uint8_t n(uint16_t opcode){
		return (opcode	& 0x000F);
	}

}

class Chip8 {
public:
	Chip8(){
		srand(time(0)); // Init RNG
	}
	uint8_t mem[MEM_SIZE] = {0}; // mem of the chip8
	uint8_t gfx[DISP_X * DISP_Y] = {0}; // 64x32 display
	uint8_t keys[NUM_KEYS] = {0};

	uint8_t stack[STACK_SIZE] = {0}; // The 64-byte stack
	uint8_t sp = 0x0; // 8-bit Stack pointer
	uint8_t v[NUM_VREGS] = {0}; // Vx registers
	uint16_t i = 0x0; // 16-bit index register. Stores memory addresses
	uint16_t pc = 0x200; // Program counter (set it to the beginning of ROM)
	uint8_t dt = 0x0; // Delay timer
	uint8_t st = 0x0; // 8-bit Sound timer
	uint16_t opcode = 0;
	bool should_draw = false; // draw flag

	bool LoadROM(const char* rom_path){
		// Get length of file 
		FILE* rom = fopen(rom_path, "rb");
		fseek(rom, 0, SEEK_END);
		size_t rom_size = ftell(rom);
		rewind(rom);
		printf("ROM size: %i Bytes\n", rom_size);
		// Allocate memory for rom
		char* rom_buf = (char*) malloc(rom_size * sizeof(char));
		// Read ROM into memory
		size_t res = fread(rom_buf, sizeof(char), rom_size, rom);
		if (!res){
			printf("Failed to load ROM\n");
			return false;
		}	
		// Load ROM into mem at 0x200
		// Most Chip-8 programs start at location 0x200 (512), but some begin at 0x600 (1536). 
		// Programs beginning at 0x600 are intended for the ETI 660 computer.
		for (int i = 0; i < rom_size; i++){
			this->mem[i + 0x200] = (uint8_t)rom_buf[i];
		}
		printf("Rom \"%s\" loaded into memory\n", rom_path);
		free(rom_buf);
		return true;
	}

					  
	// Chip-8 instructions are 2 bytes (16-bits) long 
	void cycle(){
		// Fetch the next opcode (read 16 bits)
		this->opcode = mem[this->pc] << 8 | mem[this->pc + 1];
		uint8_t op = decode(this->opcode);
		execute(op);
		this->pc += 2; // increment program counter
	}

	// Execute CPU instructions
	void execute(uint8_t op){
		const char* opstr = Op::optostr(op);
		size_t x = Op::x(opcode); // x - A 4-bit value, the lower 4 bits of the high byte of the instruction
		size_t y = Op::y(opcode); // y - A 4-bit value, the upper 4 bits of the low byte of the instruction
		uint8_t kk = Op::kk(opcode); // kk or byte - An 8-bit value, the lowest 8 bits of the instruction
		uint16_t nnn = Op::nnn(opcode); // nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
		uint8_t n = Op::n(opcode); // n or nibble - A 4-bit value, the lowest 4 bits of the instruction
		printf("0x%04x: 0x%04x\n", this->pc, opcode);
		
	}

	uint8_t decode(uint16_t opcode){
		uint8_t op;

		switch(opcode & 0xF000){
			case 0x0000:
				switch(opcode & 0x00FF){
					case 0x00E0:
						op = Op::CLS; // 00E0 no args
						break;
					case 0x00EE:
						op = Op::RET; // 0xee no args
						break;
					default:
						op = Op::SYS; // 0nnn addr
						break;
				}
				break;
			case 0x1000: 
				op = Op::JP; // 1nnn addr
				break;
			case 0x2000:
				op = Op::CALL; // 2nnn addr
				break;
			case 0x3000:
				op = Op::SE; // 3xkk Vx, byte
				break;
			case 0x4000:
				op = Op::SNE; // 4xkk Vx, byte
				break;
			case 0x6000: 
				op = Op::LD; // 6xkk Vx, byte
				break;
			case 0x7000: 
				op = Op::ADD; // 7xkk Vx, byte
				break;
			case 0x8000:
				switch(opcode & 0x000F){
					case 0x0000: // 8xy0 - LD Vx, Vy
						op = Op::LD;
						break;
					case 0x0001: // 8xy1 - OR Vx, Vy
						op = Op::OR;
						break;
					case 0x0002: // 8xy2 - AND Vx, Vy
						op = Op::AND;
						break;
					case 0x0003: // 8xy3 - XOR Vx, Vy
						op = Op::XOR;
						break;
					case 0x0004: // 8xy4 - ADD Vx, Vy
						op = Op::ADD;
						break;
					case 0x0005: // 8xy5 - SUB Vx, Vy
						op = Op::SUB;
						break;
					case 0x0006: // 8xy6 - SHR Vx {, Vy}
						op = Op::SHR;
						break;
					case 0x0007: // 8xy7 - SUBN Vx, Vy
						op = Op::SUBN;
						break;
					case 0x000E: // 8xyE - SHL Vx {, Vy}
						op = Op::SHL;
						break;
				}
				break;
			case 0x9000:
				op = Op::SNE; // 9xy0 - SNE Vx, Vy
				break;
			case 0xA000:
				op = Op::LD; // Annn - LD I, addr
				break;
			case 0xB000:
				op = Op::JP; // Bnnn - JP V0, addr
				break;
			case 0xC000:
				op = Op::RND; // Cxkk - RND Vx, byte
				break;
			case 0xD000:
				op = Op::DRW; // Vx, Vy, nibble
				break;
			case 0xE000: 
				switch(opcode & 0x00FF){
					case 0x009E:
						op = Op::SKP; // Ex9E - SKP Vx
						break;
					case 0x00A1:
						op = Op::SKNP;// ExA1 - SKNP Vx
						break;
					default:
						printf("Error: Invalid opcode: {%04x}\n", opcode);
						break;
				}
				break;
			case 0xF000:
				switch(opcode & 0x00FF){
					// Fx07 - LD Vx, DT
					case 0x0007:
						op = Op::LD;
						break;
            		// Fx0A - LD Vx, K
					case 0x000A:
						op = Op::LD;
						break;
            		// Fx15 - LD DT, Vx
					case 0x0015:
						op = Op::LD;
						break;
            		// Fx18 - LD ST, Vx
					case 0x0018: 
						op = Op::LD;
						break;
					// Fx1E - ADD I, Vx
					case 0x001E: 
						op = Op::ADD;
						break;
					// Fx29 - LD F, Vx
					case 0x0029:
						op = Op::LD;
						break;
					// Fx33 - LD B, Vx
					case 0x0033:
						op = Op::LD;
						break;
					// Fx55 - LD [I], Vx
					case 0x0055:
						op = Op::LD;
						break;
					// Fx65 - LD Vx, [I]
					case 0x0065:
						op = Op::LD;
						break;
					default:
						printf("Error: Invalid opcode: {%04x}\n", opcode);
						break;

				}
				break;
			default:
				printf("Error: Invalid opcode: {%04x}\n", opcode);
				break;
		}
		return op;
	}
	void print_registers(){
		printf("------------\n");
		printf("v registers:\n");
		for (int i = 0; i < NUM_VREGS; i++)
			printf("%i: %02x ", i, this->v[i]);
		printf("\n");
		printf("sp: 0x%04x\n", this->sp);
		printf("i: 0x%04x\n", this->i);
		printf("pc: 0x%04x\n", this->pc);
		printf("dt: 0x%02x\n", this->dt);
		printf("st: 0x%02x\n", this->st);
		printf("------------\n");
	}

	void print_stack(){
		printf("------------\n");
		printf("stack:\n");
		for (int i = 0; i < STACK_SIZE; i++)
			printf("s[%i]: 0x%02x\n", i, this->stack[i]);
		printf("------------\n");
	}
};

void print_args(uint16_t opcode, size_t x, size_t y, uint8_t kk, uint16_t nnn, uint8_t n){
	printf("opcode: 0x%04x\n", opcode);
	printf("x: 0x%01x\n", x);
	printf("y: 0x%01x\n", y);
	printf("kk: 0x%02x\n", kk);
	printf("nnn: 0x%03x\n", nnn);
	printf("n: 0x%01x\n", n);
}


int main(int argc, char *argv[]){
	const char* rom_path = argv[1];
	if (argc != 2){
		printf("Error: Need to enter the path to the file as the argument\n");
		return 0;
	}
	printf("===============START================\n");
	Chip8 chip8;
	chip8.LoadROM(rom_path);
	size_t cycles = 10;
	for (int i = 0; i < cycles; i++){
		// print_registers(cpu);
		chip8.cycle();
	}
	return 1;
}
