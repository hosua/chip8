// Sources used
// https://blog.wjdevschool.com/blog/video-game-console-emulator/
// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
// http://www.codeslinger.co.uk/pages/projects/chip8/fetchdecode.html
#include <chip8.h>
#include <display.h>
#include <cpu.h>

unsigned char textfont[80] = {
	// 1,    2, 	3, 	  4, 	5 bytes
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

const char* Op::optostr(uint8_t op);

// Chip8 is Big-endian
// Big-endian reads from msb -> lsb
// Examples: 3xkk, 5xy0
// These extract the bits, not the values at the register
// x = 4-bit index of the V register (V0-VF)
size_t Op::x(uint16_t opcode){
	return (opcode & 0x0F00) >> 8; 
}
// y = 4-bit index of the V register (V0-VF)
size_t Op::y(uint16_t opcode){
	return (opcode & 0x00F0) >> 4;
}
// kk = 8-bit constant
uint8_t Op::kk(uint16_t opcode){
	return (opcode & 0x00FF);
}
// nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
uint16_t Op::nnn(size_t opcode){
	return (opcode & 0x0FFF);
}
// n or nibble - A 4-bit value, the lowest 4 bits of the instruction
uint8_t Op::n(uint16_t opcode){
	return (opcode	& 0x000F);
}

const char* Op::optostr(uint8_t op){
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
		default: return "UNK";
	}
	return "UWOTM8";
}



bool Chip8::LoadROM(const char* rom_path){
	// Get length of file 
	FILE* rom = fopen(rom_path, "rb");
	fseek(rom, 0, SEEK_END);
	size_t rom_size = ftell(rom);
	rewind(rom);
	printf("ROM size: %i Bytes\n", rom_size);
	// Allocate memory for rom 
	uint8_t* rom_buf = (uint8_t*) malloc(rom_size);
	// Read ROM into memory
	size_t res = fread(rom_buf, 1, rom_size, rom);
	if (!res){
		printf("Failed to load ROM\n");
		return false;
	}	
	// Load ROM into mem at 0x200
	// Most Chip-8 programs start at location 0x200 (512), but some begin at 0x600 (1536). 
	// Programs beginning at 0x600 are intended for the ETI 660 computer.
	for (int i = 0; i < rom_size; i++){
		this->mem[0x200 + i] = (uint8_t)rom_buf[i];
	}
	free(rom_buf);
	printf("Rom \"%s\" loaded into memory\n", rom_path);
	return true;
}

// Load font set into memory
void Chip8::LoadFont(uint8_t* font){
	for (int i = 0; i < 0x80; i++){
		this->mem[i] = font[i];
	}
}

void Chip8::print_display(){
	for (int i = 0; i < DISP_X*DISP_Y; i++){
		if (gfx[i] == 1){
			printf("%s", PX);
		}		
		if (((i+1) % DISP_X) == 0){
			printf("\n");
		}
	}
}

void Chip8::fill_gfx(){
	for (int i = 0; i < DISP_X*DISP_Y; i++){
		this->gfx[i] = 1;
	}
}

void Chip8::print_test(){
	fill_gfx();
	print_display();
}


