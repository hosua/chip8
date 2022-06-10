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

// MEM_SIZE is how much RAM the chip8 can access
#define MEM_SIZE 4096

using std::cout;
using std::endl;
using std::string;



// Note that KK is synonymous with NN
namespace Op {
	enum { CLS, RET, SYS, JP, CALL, 
			SE, SNE, LD, ADD, LDR, 
			OR, AND, XOR, SUB, SHR, 
			SUBN, SHL, RND, DRW, SKP,
			SKNP,
	};

	const char* optostring(uint8_t op){
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
		return "You fucked up kid\n";
	}
	// Chip8 uses Big-endian
	// Big-endian reads from msb -> lsb
	// Examples: 3xkk, 5xy0
	// x = 4-bit index of the V register (V0-VF)
	// y = 4-bit index of the V register (V0-VF)
	// kk = 8-bit constant
	size_t x(uint16_t opcode){
		return (opcode & 0x0F00) >> 8; 
	}
	size_t y(uint16_t opcode){
		return (opcode & 0x00F0) >> 4;
	}
	uint8_t kk(uint16_t opcode){
		return (opcode & 0x00FF);
	}
	uint16_t nnn(size_t opcode){
		return (opcode & 0x0FFF);
	}

}

class Chip8 {
public:
	uint8_t mem[MEM_SIZE] = {0};

	std::ifstream::pos_type GetFilesize(const char* filename){
		std::ifstream file_stream(filename, std::ifstream::ate | std::ifstream::binary);
		return file_stream.tellg();
	}

	// https://stackoverflow.com/questions/15138785/how-to-read-a-file-into-vector-in-c
	std::vector<uint8_t> GetROMData(const char* rom_file){
		size_t rom_size = GetFilesize(rom_file);
		std::ifstream file_stream(rom_file, std::ios::in | std::ios::binary);
		std::istream_iterator<uint8_t> start(file_stream), end;
		std::vector<uint8_t> rom_data(start, end);
		return rom_data;
	}

	void LoadRom(const char* rom_file){
		std::vector<uint8_t> rom_data;
		// Get length of file 
		int rom_size = std::filesystem::file_size(rom_file);
		cout << "ROM size: " << rom_size << endl;

		rom_data = GetROMData(rom_file);
		
		// Load ROM into main memory at 0x200
		// Most Chip-8 programs start at location 0x200 (512), but some begin at 0x600 (1536). 
		// Programs beginning at 0x600 are intended for the ETI 660 computer.
		for (int i = 0; i < rom_size; i++){
			this->mem[0x200 + i] = rom_data[i];
		}
		printf("Rom \"%s\" loaded into memory\n", rom_file);
	}

};


// There are two things we need to emulate with regard to the CPU regardless of console or architecture:
// 1) CPU registers 
// 2) CPU instructions
// More info here in section 2.2: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
class CPU {
public:
	uint8_t v[16] = {0x0}; // Vx registers
	uint16_t i = 0x0; // Stores memory addresses
	uint16_t pc = 0x200; // Program counter (set it to the beginning of ROM)
	uint8_t sp = 0x0; // Stack pointer
	uint8_t dt = 0x0; // Delay timer
	uint8_t st = 0x0; // Sound timer
					  
	// Chip-8 instructions are 2 bytes (16-bits) long 
	void cycle(Chip8 chip8){
		uint8_t *memory = chip8.mem;
		// Get the next opcode (read 16 bits)
		uint16_t opcode = memory[this->pc] << 8 | memory[this->pc + 1];

		// Increment to next opcode
	 	this->pc += 2;

		operate(opcode);
	}

	// Perform CPU instructions
	void operate(uint16_t opcode){
		uint16_t op = decode(opcode);
		const char* opstr = Op::optostring(op);

		size_t x = Op::x(opcode);
		size_t y = Op::y(opcode);
		uint8_t kk = Op::kk(opcode);
		uint16_t nnn = Op::nnn(opcode);

		switch(op){
			case Op::SYS: // Ignored
				printf("Ignoring SYS op\n");
				break;
			case Op::JP: 
				printf("addr: %04x\n", nnn);
				switch(opcode & 0xF000){
					case 0x1000: // 1nnn - jump to address nnn
						this->pc = nnn;
						break;
					case 0xB000: // Bnnn - jump to address nnn + v[0]
						this->pc = nnn + this->v[0];
						break;
				}
				break;
			case Op::LD: 	
				switch(opcode & 0xF000){
					case 0x6000: // 6xkk - Set Vx to kk
						this->v[x] = kk;
						break;
					case 0x8000: // 8xy0 - Set Vx to Vy
						this->v[x] = this->v[y];
						break;
					case 0xA000: // annn - Set i to address nnn
						this->i = nnn;
						break;
					case 0xF000:
						switch(opcode & 0x00FF){
							case 0x0007: // Fx07 - LD Vx, DT "load Vx into DT"
								this->dt = this->v[x];
								break;
							case 0x000A: // Fx0A - LD Vx, K
								// this->v[x] = get_key();
								break;
							case 0x0015: // Fx15 - LD DT, Vx
								this->v[x] = this->dt;
								break;
							case 0x0018: // Fx18 - LD ST, Vx
								this->v[x] = this->st;
								break;
							case 0x001E: // Fx1E - ADD I, Vx
								this->v[x] += this->i;
								break;
							case 0x0029: // Fx29 - LD F, Vx
								// The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. 
								this->i = this->v[x];
								break;
							case 0x0033: // Fx33 - LD B, Vx
								// TODO
								// Store BCD representation of Vx in memory locations I, I+1, and I+2.
								// BCD = Binary coded representation, see https://www.techtarget.com/whatis/definition/binary-coded-decimal
								break;
							case 0x0055: // Fx55 - LD [I], Vx
								// Store registers V0 through Vx in memory starting at location I (without modifications to I).
								break;
							case 0x0065: // Fx65 - LD Vx, [I]
								// Read from registers V0 through Vx in memory starting at location I.
								break;	
						}
				}
				break;
			case Op::ADD: 	// Add kk to Vx
				this->v[x] += kk;
				break;
			default:
				printf("Error: Invalid opcode {%04x}\n", opcode);
		}
		printf("0x%04x: 0x%04x %s\n", this->pc, opcode, opstr);
	}

	uint8_t decode(uint16_t opcode){
		uint8_t op;
		// opcode & 0x000F extracts the value at F
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
};


int main(int argc, char *argv[]){
	const char* rom_path = argv[1];
	if (argc != 2){
		printf("Error: Need to enter the path to the file as the argument\n");
		return 0;
	}
	printf("===============START================\n");
	Chip8 chip8;
	// chip8.LoadRom("chip8-test-rom", "test_opcode.ch8");
	chip8.LoadRom(rom_path);
	CPU cpu;
	for (int i = 0; i < 0xFF/2; i++){
		cpu.cycle(chip8);
	}
	return 1;
}
