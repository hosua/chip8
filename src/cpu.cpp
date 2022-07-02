#include <cpu.h>
#include <chip8.h>
#include <input.h>


// Chip-8 instructions are 2 bytes (16-bits) long 
void CPU::cycle(){
	// Fetch the next opcode (read 16 bits)
	this->opcode = mem[pc] << 8 | mem[pc + 1];
	uint8_t op = decode(this->opcode);
	execute(op);
	pc += 2; // increment program counter
}

// Counts down dt when it is non-zero (60Hz, i.e. 1/60 seconds per tick)
void CPU::delay_timer(){
	if (this->dt){
		clock->wait(this->dt); // Wait for ticks to process
		this->dt = 0x0; // set dt to 0
	}
	if (!SLOW_MODE)
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	else 
		std::this_thread::sleep_for(std::chrono::microseconds(TICK * 2));
}

uint8_t CPU::decode(uint16_t opcode){
	uint8_t op = Op::ERR;
	switch(opcode & 0xF000){
		case 0x0000:
			switch(opcode & 0x00FF){
				case 0x00E0: // 00E0 no args
					op = Op::CLS; 
					break;
				case 0x00EE: // 0xEE no args
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
		case 0x5000: // 5xy0 - SE Vx, Vy
			op = Op::SE; 
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
					if (VERBOSE_CPU) printf("Error: Invalid opcode: {%04X}\n", opcode);
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
					if (VERBOSE_CPU) printf("Error: Invalid opcode: {%04X}\n", opcode);
					break;

			}
			break;
		default:
			if (VERBOSE_CPU) printf("Error: Invalid opcode: {%04X}\n", opcode);
			break;
	}
	return op;
}

// Execute CPU instructions
void CPU::execute(uint8_t op){
	const char* description = "";
	const char* opstr = Op::optostr(op);
	size_t x = Op::x(opcode); // x - A 4-bit value, the lower 4 bits of the high byte of the instruction
	size_t y = Op::y(opcode); // y - A 4-bit value, the upper 4 bits of the low byte of the instruction
	uint8_t kk = Op::kk(opcode); // kk or byte - An 8-bit value, the lowest 8 bits of the instruction
	uint16_t nnn = Op::nnn(opcode); // nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
	uint8_t n = Op::n(opcode); // n or nibble - A 4-bit value, the lowest 4 bits of the instruction
	switch(op){
		default:
			if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04X}\n", opcode);
			break;
		case Op::ADD: // Add kk to Vx
			{
				switch(opcode & 0xF000){
					case 0x7000: // ADD Vx, byte
						description = "Vx, kk";
						v[x] += kk;
						break;
					case 0x8000: // 8xy4 ADD Vx, Vy
						{
							description = "Vx, Vy";
							v[x] += v[y];
							if (v[y] > v[x]){ 
								// Carry flag
								v[0xF] = 1; 	
							} else {
								v[0xF] = 0;
							}
							break;
						}
					case 0xF000: // Fx1E ADD I = I + Vx
						description = "I, Vx";
						this->i += v[x];
						break;
				}
				break;
			}
		case Op::AND: // 8xy2 - AND Vx, Vy
					  // Set Vx = Vx AND Vy.
			description = "Vx, Vy";
			v[x] &= v[y];
			break;
		case Op::CALL: // 2nnn - Call subroutine
			stack.push(pc);
			pc = nnn; // Store address into program counter
			pc -= 2;
			break;
		case Op::CLS: // 0x00E0 - Clear screen
					  // Clear 64x32 display
			for(int i = 0; i < DISP_X*DISP_Y; i++)
				chip8->gfx[i] = 0;
			chip8->draw_flag = true;
			break;
		case Op::JP: // Jump
			{
				switch(opcode & 0xF000){
					default:
						if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04X}\n", opcode);
						break;
					case 0x1000: // 1nnn - jump to address nnn
						description = "addr";
						pc = nnn;
						pc -= 2;
						break;
					case 0xB000: // Bnnn - jump to address nnn + v[0]
						description = "V0, addr";
						pc = nnn + v[0];
						pc -= 2;
						break;
				}
				break;
			}
		case Op::DRW: // Dxyn - Draw
			{
				description = "Vx, Vy, nibble";
				/* Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
				 * Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not 
				 * change after the execution of this instruction. As described above, VF is set to 1 if any 
				 * screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen 
				 */
				// Dxyn
				// Each sprite will always be 8 pixels wide
				// The nibble, n is the height we will draw */
				v[0xF] = 0;
				// Here, we need to get the actual values from the V registers 
				// This is different from the x,y functions defined in Op::, as those extract the bits from the opcode itself.
				n = opcode & 0x000F;
				uint8_t px;
				// dy is the y position of the line being drawn
				for (int dy = 0; dy < n; dy++){
					px = mem[this->i + dy];
					for(int dx = 0; dx < 8; dx++){
						// dx is the x position of the pixel in the line being drawn
						if(px & (0x80 >> dx)){
							// and if gfx is set
							if(chip8->gfx[(v[x] + dx + ((v[y] + dy) * DISP_X))]){
								// Set VF flag to 1 indicating that at least one pixel was unset
								v[0xF] = 1;
							}
							chip8->gfx[v[x] + dx + ((v[y] + dy) * DISP_X)] ^= 1;
						}
					}
				}

				chip8->draw_flag = true;
				break;
			}
		case Op::LD: 	
			{
				switch(opcode & 0xF000){
					default:
						if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04X}\n", opcode);
						break;
					case 0x6000: // 6xkk - Set Vx to kk
						description = "Vx, kk";
						v[x] = kk;
						// pc -= 2; // TODO: I don't know how this is right, but I fucking want to know why
						break;
					case 0x8000: // 8xy0 - Set Vx to Vy
						description = "Vx, Vy";
						v[x] = v[y];
						break;
					case 0xA000: // Annn - Set i to address nnn
						description = "I, addr";
						this->i = nnn;
						break;
					case 0xF000:
						switch(opcode & 0x00FF){
							default:
								if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04X}\n", opcode);
								break;
							case 0x0007: // Fx07 - LD Vx, DT "load DT into Vx"
								description = "Vx, DT";
								v[x] = dt;
								break;
							case 0x000A: // Fx0A - LD Vx, K
								description = "Vx, K";
								v[x] = InputHandler::WaitForKeyPress();
								break;
							case 0x0015: // Fx15 - LD DT, Vx
								description = "DT, Vx";
								dt = v[x];
								break;
							case 0x0018: // Fx18 - LD ST, Vx
								description = "ST, Vx";
								st = v[x];
								break;
							case 0x0029: // Fx29 - LD F, Vx
								description = "F, Vx";
								// The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. 
								this->i = v[x] * 0x5; // Each font is 5 bytes wide (as shown in textfont) 
								break;
							case 0x0033: // Fx33 - LD B, Vx
								description = "B, Vx";
								// Store BCD representation of Vx in mem locations i, i+1, and I+2.
								// BCD = Binary coded representation, see https://www.techtarget.com/whatis/definition/binary-coded-decimal
								mem[this->i] = v[x] / 100; // Load 100s place into memory
								mem[this->i+1] = (v[x] / 10) % 10; // Load 10s place into memory
								mem[this->i+2] = v[x] % 10; // Load 1s place into memory
								break;
							case 0x0055: // Fx55 - LD [I], Vx
								description = "[I], Vx";
								// TODO: WRONG??
								for (uint8_t i = 0; i <= x; i++){
									// Stores from V0 to VX (including VX) into memory, starting at address I. The offset from I is increased by 1 for each value written, 
									// but I itself is left unmodified.
									mem[this->i + i] = v[i];
								}
								this->i += x + 1;
								break;
							case 0x0065: // Fx65 - LD Vx, [I]
								description = "Vx [I]";
								// Read from memory starting at address I into v registers
								for (uint8_t i = 0; i <= x; i++){
									v[i] = mem[this->i + i];
								}

								this->i += x + 1;
								break;	
						}
				}
				break;
			}
		case Op::OR: // 8xy1 - OR Vx, Vy
			description = "Vx, Vy";
			// Set Vx = Vx OR Vy.
			v[x] |= v[y];
			break;
		case Op::SE: // 5xy0 - SE Vx, Vy
			switch(opcode & 0xF000){
				default:
					if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04X}\n", opcode);
					break;
					// 3xkk - SE Vx, byte
				case 0x3000:
					description = "Vx, kk";
					// The interpreter compares register Vx to kk, and if they are equal, 
					// increments the program counter by 2.
					if (v[x] == kk)
						pc += 2;
					break;
					// 5xy0 - SE Vx, Vy
				case 0x5000:
					description = "Vx, Vy";
					// Skip next instruction if Vx = Vy.
					if (v[x] == v[y])
						pc += 2;
					break;
			}
			break;
		case Op::SNE:
			switch (opcode & 0xF000){
				// 4xkk - SNE Vx, byte
				case 0x4000:
					description = "Vx, kk";
					// Skip next instruction if Vx != kk
					if (v[x] != kk)
						pc += 2;
					break;
					// 9xy0 - SNE Vx, Vy
				case 0x9000: 
					description = "Vx, Vy";
					// 9xy0 - SNE Vx, Vy
					// Skip next instruction if Vx != Vy.
					if (v[x] != v[y])
						pc += 2;
					break;
			}
			break;
		case Op::SHL: // 8xyE - SHL Vx {, Vy}
			description = "Vx {, Vy}";
			// The 0-based index of the msb in an 8-bit number is 7.
			v[0xF] = v[x] >> MSB_POS;
			// Shift vy left once and store it in vx
			v[x] <<= 1;
			break;
		case Op::SHR: // 8xy6 - SHR Vx {, Vy}
			description = "Vx {, Vy}";
			// Set Vx = Vx SHR 1.
			// Store LSB in vf
			v[0xF] = v[x] & 1;
			// Shift right once and store it in vx
			v[x] >>= 1;
			break;
		case Op::SKP: // Ex9E - SKP Vx "Skip if pressed"
			description = "Vx";
			// Skip next instruction if key with value of Vx is pressed
			if (chip8->keys[v[x]]){ // If key is pressed
				pc += 2;
				if (VERBOSE_INPUT) InputHandler::PrintChip8Keys(chip8);
			} 
			break;
		case Op::SKNP: // ExA1 - SKNP Vx "Skip if not pressed"
			description = "Vx";
			if (!chip8->keys[v[x]]) // If key is not pressed
				pc += 2;
			break;
		case Op::SUB: // 8xy5 - SUB Vx, Vy
			description = "Vx, Vy";
			// If Vx > Vy, then VF is set to 1, otherwise 0. 
			if (v[x] > v[y])
				v[0xF] = 1; 	
			else
				v[0xF] = 0;
			// Set Vx = Vx - Vy, set VF = NOT borrow.
			// Then Vy is subtracted from Vx, and the results stored in Vx.
			v[x] -= v[y];
			break;
		case Op::SUBN: // 8xy7 - SUBN Vx, Vy
			description = "Vx, Vy";
			// Set Vx = Vy - Vx, set VF = NOT borrow.
			// If Vy > Vx, then VF is set to 1, otherwise 0. 
			if (v[y] > v[x])
				v[0xF] = 1;
			else
				v[0xF] = 0;
			// Then Vx is subtracted from Vy, and the results stored in Vx.
			v[x] = v[y] - v[x];
			break;
		case Op::RET: // 00EE - RET "Return"
			pc = stack.top();
			stack.pop();
			break;
		case Op::RND: // RND Vx, byte
			description = "Vx, kk";
			v[x] = (rand() % 0xFF) & 0xFF; // Set Vx to random # from (0-255), then & 255
			break;
		case Op::SYS: // Ignored
			break;
		case Op::XOR: // 8xy3 - XOR Vx, Vy
			description = "Vx, Vy";
			// Performs a bitwise XOR on the values of Vx and Vy, then stores the result in Vx.
			v[x] ^= v[y];
			break;
		case Op::ERR:
			if (VERBOSE_CPU) printf("You fucked up kid\n");
			break;
	}
	if (VERBOSE_CPU) printf("0x%04X: 0x%04X %s %s\n", pc, opcode, opstr, description);
	print_args(opcode);
	print_registers();
}
/* debugging functions */
void CPU::print_registers(){
	printf("------------\n");
	printf("v registers:\n");
	for (int i = 0; i < NUM_VREGS; i++)
		printf("%02X ", i);
	printf("\n");
	for (int i = 0; i < NUM_VREGS; i++)
		printf("%02X ", v[i]);
	printf("\n");
	// printf("sp: 0x%04X\n", this->sp);
	printf("i: 0x%04X\n", this->i);
	printf("pc: 0x%04X\n", pc);
	printf("dt: 0x%02X\n", this->dt);
	printf("st: 0x%02X\n", this->st);
	printf("------------\n");
}

void CPU::print_args(uint16_t opcode){
	printf("------------\n");
	printf("op: %s\n", Op::optostr(decode(opcode)));
	size_t x = Op::x(opcode);
	size_t y = Op::y(opcode);
	uint8_t kk = Op::kk(opcode);
	uint16_t nnn = Op::nnn(opcode);
	uint8_t n = Op::n(opcode);
	printf("x: 0x%02zX\n", x);
	printf("y: 0x%02zX\n", y);
	printf("kk: 0x%02X\n", kk);
	printf("nnn: 0x%03X\n", nnn);
	printf("n: 0x%01X\n", n);
}

