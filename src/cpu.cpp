#include <cpu.h>
#include <chip8.h>

#include <cstdint> 
#include <cstdio>

// Chip-8 instructions are 2 bytes (16-bits) long 
void CPU::cycle(){
	// Fetch the next opcode (read 16 bits)
	this->opcode = mem[this->pc] << 8 | mem[this->pc + 1];
	uint8_t op = decode(this->opcode);
	execute(op);
	this->pc += 2; // increment program counter
}

uint8_t CPU::decode(uint16_t opcode){
	uint8_t op;
	uint8_t x = Op::x(opcode);
	uint8_t y = Op::y(opcode);
	uint8_t n = Op::n(opcode);
	size_t nnn = Op::nnn(opcode);

	switch(opcode & 0xF000){
		case 0x0000:
			switch(opcode & 0x00FF){
				case 0x00E0: // 00E0 no args
					op = Op::CLS; 
					break;
				case 0x00EE: // 0xee no args
					op = Op::RET; 
					break;
				default: // 0nnn addr
					op = Op::SYS; 
					break;
			}
			break;
		case 0x1000: // 1nnn addr
			op = Op::JP; 
			break;
		case 0x2000: // 2nnn addr
			op = Op::CALL; 
			break;
		case 0x3000: // 3xkk Vx, byte
			op = Op::SE; 
			break;
		case 0x4000: // 4xkk Vx, byte
			op = Op::SNE; 
			break;
		case 0x6000: // 6xkk Vx, byte
			op = Op::LD; 
			break;
		case 0x7000: // 7xkk Vx, byte
			op = Op::ADD; 
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
			op = Op::DRW; // Dxyn - Vx, Vy, nibble 
			break;
		case 0xE000: 
			switch(opcode & 0x00FF){
				case 0x009E: // Ex9E - SKP Vx
					op = Op::SKP; 
					break;
				case 0x00A1: // ExA1 - SKNP Vx
					op = Op::SKNP;
					break;
				default:
					printf("Error: Invalid opcode: {%04x}\n", opcode);
					break;
			}
			break;
		case 0xF000:
			switch(opcode & 0x00FF){
				case 0x0007: // Fx07 - LD Vx, DT
				case 0x000A: // Fx0A - LD Vx, K
				case 0x0015: // Fx15 - LD DT, Vx
				case 0x0018: // Fx18 - LD ST, Vx
				case 0x0033: // Fx33 - LD B, Vx
				case 0x0029: // Fx29 - LD F, Vx
				case 0x0055: // Fx55 - LD [I], Vx
				case 0x0065: // Fx65 - LD Vx, [I]
					op = Op::LD;
					break;
				case 0x001E: // Fx1E - ADD I, Vx
					op = Op::ADD;
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


// Execute CPU instructions
void CPU::execute(uint8_t op){
	const char* opstr = Op::optostr(op);
	size_t x = Op::x(opcode); // x - A 4-bit value, the lower 4 bits of the high byte of the instruction
	size_t y = Op::y(opcode); // y - A 4-bit value, the upper 4 bits of the low byte of the instruction
	uint8_t kk = Op::kk(opcode); // kk or byte - An 8-bit value, the lowest 8 bits of the instruction
	uint16_t nnn = Op::nnn(opcode); // nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
	uint8_t n = Op::n(opcode); // n or nibble - A 4-bit value, the lowest 4 bits of the instruction
	uint8_t px; // For draw
	printf("0x%04x: 0x%04x ", this->pc, opcode);
	printf("%s ", opstr);
	switch(op){
		default:
			printf("\nError: Invalid opcode {%04x}\n", opcode);
			break;
		case Op::ADD: 	// Add kk to Vx
			switch(opcode & 0xF000){
				case 0x7000: // ADD Vx, byte
					this->v[x] += kk;
					printf("ADD 0x%02x -> V%zu\n", kk, x);
					// print_registers();
					break;
				case 0x8000: // 8xy4 ADD Vx, Vy
					this->v[y] += this->v[x]; 
					printf("V%i V%i\n", this->v[x], this->v[y]);
					break;
				case 0x001E: // Fx1E ADD I = I + Vx
					this->i += this->v[x];
					printf("I, V%zu\n", x);
					break;
			}
			break;
		case Op::CALL: // Call subroutine
			this->stack[this->sp] = this->pc; // Put pc on top of the stack
			this->sp++; // Increment stack pointer
			this->pc = nnn; // Store address into program counter
			this->pc -= 2;
			break;
		case Op::CLS: // Clear screen
			printf("%s\n", opstr);
			// Clear 64x32 display
			for(int i = 0; i < DISP_X*DISP_Y; i++)
				chip8->gfx[i] = 0;
			chip8->draw_flag = true;
			break;
		case Op::JP: 
			switch(opcode & 0xF000){
				default:
					printf("\nError: Invalid opcode {%04x}\n", opcode);
					break;
				case 0x1000: // 1nnn - jump to address nnn
					this->pc = nnn;
					printf("0x%03x\n", nnn);
					this->pc -= 2;
					break;
				case 0xB000: // Bnnn - jump to address nnn + v[0]
					this->pc = nnn + v[0];
					printf("0x%03x + 0x%03x\n", v[0], nnn);
					this->pc -= 2;
					break;
			}
			break;
		case Op::DRW:
		{
			printf("Vx: 0x%01x Vy: 0x%01x n: %i i: 0x%02x\n", this->v[x], this->v[y], n, this->i);
			/* Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
			 * Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not 
			 * change after the execution of this instruction. As described above, VF is set to 1 if any 
			 * screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen 
			 */
			// Dxyn
			// Each sprite will always be 8 pixels wide
			// The nibble, n is the height we will draw */
			this->v[0xF] = 0;
			// Here, we need to get the actual values from the V registers 
			// This is different from the x,y functions defined in Op::, as those extract the bits from the opcode itself.
			n = opcode & 0x000F;
			uint8_t px;
			// dy is the y position of the line being drawn
			for (int dy = 0; dy < n; dy++){
				px = this->mem[this->i + dy];
				for(int dx = 0; dx < 8; dx++){
					// dx is the x position of the pixel in the line being drawn
					if(px & (0x80 >> dx)){
						// and if gfx is set
						if(chip8->gfx[(this->v[x] + dx + ((this->v[y] + dy) * 64))]){
							// Set VF flag to 1 indicating that at least one pixel was unset
							this->v[0xF] = 1;
						}
						chip8->gfx[this->v[x] + dx + ((this->v[y] + dy) * 64)] ^= 1;
					}
				}
			}

			chip8->draw_flag = true;
			break;
		}
		case Op::LD: 	
			switch(opcode & 0xF000){
				default:
					printf("\nError: Invalid opcode {%04x}\n", opcode);
					break;
				case 0x6000: // 6xkk - Set Vx to kk
					this->v[x] = kk;
					printf("V%zu 0x%02x\n", x, kk);
					this->pc -= 2; // TODO: I don't know how this is right, but I fucking want to know why
					break;
				case 0x8000: // 8xy0 - Set Vx to Vy
					this->v[x] = this->v[y];
					printf("V%i V%i\n", this->v[x], this->v[y]);
					break;
				case 0xA000: // annn - Set i to address nnn
					this->i = nnn;
					printf("0x%03x -> I\n", nnn);
					break;
				case 0xF000:
					switch(opcode & 0x00FF){
						default:
							printf("\nError: Invalid opcode {%04x}\n", opcode);
							break;
						case 0x0007: // Fx07 - LD Vx, DT "load Vx into DT"
							this->dt = this->v[x];
							printf("V%zu DT\n", x);
							break;
						case 0x000A: // Fx0A - LD Vx, K
									 // this->v[x] = get_key();
							printf("%s Vx, k NOT IMPLEMENTED\n", opstr);
							break;
						case 0x0015: // Fx15 - LD DT, Vx
							this->v[x] = this->dt;
							printf("V%zu\n", x);
							break;
						case 0x0018: // Fx18 - LD ST, Vx
							this->v[x] = this->st;
							printf("V%zu\n", x);
							break;
						case 0x0029: // Fx29 - LD F Vx -> I
									 // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. 
							printf("F V%zu -> I\n", x);
							this->i = this->v[x] * 0x5; // Each font is 5 bytes wide (as shown in textfont) 
							break;
						case 0x0033: // Fx33 - LD B, Vx
									 // TODO
									 // Store BCD representation of Vx in mem locations I, I+1, and I+2.
									 // BCD = Binary coded representation, see https://www.techtarget.com/whatis/definition/binary-coded-decimal
							printf("LD B, V%zu\n", x);
							this->mem[this->i] = this->v[x] / 100; // Load 100s place into memory
							this->mem[this->i+1] = (this->v[x] / 10) % 10; // Load 10s place into memory
							this->mem[this->i+2] = this->v[x] % 10; // Load 1s place into memory
							break;
						case 0x0055: // Fx55 - LD [I], Vx
							for (int i = 0; i <= x; i++){
								// Stores from V0 to VX (including VX) into memory, starting at address I. The offset from I is increased by 1 for each value written, 
								// but I itself is left unmodified.
								printf("I -> V%zu\n", x);
								this->mem[this->i + i] = this->v[i];
							}
							this->i += x + 1;
							break;
						case 0x0065: // Fx65 - LD Vx, [I]
									 // Read from memory starting at address I into v registers
							printf("LD V0-V%zu -> I\n", x);
							for (int i = 0; i <= x; i++){
								this->v[i] = this->mem[this->i + i];
							}

							this->i += x + 1;
							break;	
					}
			}
		case Op::SE:
			// The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
			if (this->v[x] == kk){ 
				// Skip instruction
				this->pc += 2;
				printf("V%zu: 0x%02x == 0x%02x\n", x, this->v[x], kk);
			} else {
				// Do not skip instruction
				printf("V%zu: 0x%02x != 0x%02x\n", x, this->v[x], kk);
			}
			break;
		case Op::RET:
			printf("top: 0x%04x\n", stack[sp]);
			this->sp--;
			this->pc = stack[sp];
			// printf("WARNING: 0xEE RET is not implented yet!\n");
			break;
		case Op::RND: // RND Vx 
			this->v[x] = (rand() % 0xFF) & 0xFF; // Set Vx to random # from (0-255), then & 255
			printf("RND V%zu = 0x%02x\n", x, v[x]);
			break;
		case Op::SYS: // Ignored
			printf("\n");
			break;
		case Op::XOR: // Exclusive OR
			// Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
			printf("V%zu = Vx ^ Vy = 0x%02x\n", x, this->v[x] ^ this->v[y]);
			this->v[x] ^= this->v[y];
			break;
	}
}
/* debugging functions */
void CPU::print_registers(){
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

void CPU::print_stack(){
	printf("------------\n");
	printf("stack:\n");
	for (int i = 0; i < STACK_SIZE; i++)
		printf("s[%i]: 0x%02x\n", i, this->stack[i]);
	printf("------------\n");
}
void print_args(uint16_t opcode, size_t x, size_t y, uint8_t kk, uint16_t nnn, uint8_t n){
	printf("opcode: 0x%04x\n", opcode);
	printf("x: %zu\n", x);
	printf("y: %zu\n", y);
	printf("kk: 0x%02x\n", kk);
	printf("nnn: 0x%03x\n", nnn);
	printf("n: 0x%01x\n", n);
}

