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


// Note that KK is synonymous with NN
namespace Opcode {
	enum { CLS, RET, SYS, JP, CALL, 
			SE, SNE, LD, ADD, LDR, 
			OR, AND, XOR, SUB, SHR, 
			SUBN, SHL, RND, DRW,
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
		}
		return "NAN";
	}
	// TODO: we still need to extract the arguments from the opcodes (that require them obviously)
	uint8_t decode(uint16_t opcode){
		uint8_t op;
		// opcode & 0x000F extracts the value at F
		switch(opcode & 0xF000){
			case 0x0000:
					// 00E0 - CLS
					// 00EE - RET
					// 0nnn - SYS addr
				break;
			case 0x1000: 
				op = JP; // 1nnn addr
				break;
			case 0x6000: 
				op = LD; // 6xkk Vx, byte
				break;
			case 0x7000: 
				op = ADD; // 7xkk Vx, byte
				break;
			case 0x8000:
				switch(opcode & 0x000F){
					case 0x0: // 8xy0 - LD Vx, Vy
						op = LD;
						break;
					case 0x1: // 8xy1 - OR Vx, Vy
						op = OR;
						break;
					case 0x2: // 8xy2 - AND Vx, Vy
						op = AND;
						break;
					case 0x3: // 8xy3 - XOR Vx, Vy
						op = XOR;
						break;
					case 0x4: // 8xy4 - ADD Vx, Vy
						op = ADD;
						break;
					case 0x5: // 8xy5 - SUB Vx, Vy
						op = SUB;
						break;
					case 0x6: // 8xy6 - SHR Vx {, Vy}
						op = SHR;
						break;
					case 0x7: // 8xy7 - SUBN Vx, Vy
						op = SUBN;
						break;
					case 0xE: // 8xyE - SHL Vx {, Vy}
						op = SHL;
						break;
				}
				break;
			default:
				printf("Error: Invalid opcode: {%04x}\n", opcode);
				break;
		}
		return op;
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

	void LoadRom(const char* path, const char* rom_file){
		std::filesystem::current_path(path);
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

		std::filesystem::current_path("..");
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
		// Get the next opcode
		uint16_t opcode = memory[this->pc] << 8 | memory[this->pc + 1];

		uint16_t op = Opcode::decode(opcode);
		const char* opstr = Opcode::optostring(op);
		printf("0x%04x: 0x%04x %s\n", this->pc, opcode, opstr);

		// Increment to next opcode
	 	this->pc += 2;

		size_t x = Opcode::x(op);
		size_t y = Opcode::y(op);
		uint8_t kk = Opcode::kk(op);
		// Perform CPU instructions
		uint16_t nnn;
		switch(op){
			case Opcode::JP:
				nnn = Opcode::nnn(opcode);
				printf("addr: %04x\n", nnn);
				this->pc = nnn;
				break;
			case Opcode::LD: 	// Set VX to KK
				this->v[x] = kk;
				break;
			case Opcode::ADD: 	// Add KK to VX
				this->v[x] += kk;
				break;
			case Opcode::LDR: 	// Sets VX to value of VY
				this->v[x] = v[y];
				break;
		}
	}
};


int main(){
	printf("===============START================\n");
	Chip8 chip8;
	chip8.LoadRom("games", "BLINKY");
	CPU cpu;
	for (int i = 0; i < 0xFF/2; i++){
		cpu.cycle(chip8);
	}
}
