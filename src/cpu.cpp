#include <cpu.h>
#include <chip8.h>
#include <input.h>

// For timer
#ifdef __linux__
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

// Chip-8 instructions are 2 bytes (16-bits) long 
void CPU::cycle(){
	// Fetch the next opcode (read 16 bits)
	this->opcode = mem[this->pc] << 8 | mem[this->pc + 1];
	uint8_t op = decode(this->opcode);
	execute(op);
	this->pc += 2; // increment program counter
}

// Counts down dt when it is non-zero (60Hz, i.e. 1/60 seconds per tick)
void CPU::delay_timer(){
	if (this->dt){
		clock->wait(this->dt); // Wait for ticks to process
		this->dt = 0x0; // set dt to 0
	}
}

uint8_t CPU::decode(uint16_t opcode){
	uint8_t op = Op::ERR;
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
					if (VERBOSE_CPU) printf("Error: Invalid opcode: {%04x}\n", opcode);
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
					if (VERBOSE_CPU) printf("Error: Invalid opcode: {%04x}\n", opcode);
					break;

			}
			break;
		default:
			if (VERBOSE_CPU) printf("Error: Invalid opcode: {%04x}\n", opcode);
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
	if (VERBOSE_CPU) printf("0x%04x: 0x%04x ", this->pc, opcode);
	if (VERBOSE_CPU) printf("%s ", opstr);
	switch(op){
		default:
			if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04x}\n", opcode);
			break;
		case Op::ADD: 	// Add kk to Vx
			switch(opcode & 0xF000){
				case 0x7000: // ADD Vx, byte
					this->v[x] += kk;
					if (VERBOSE_CPU) printf("ADD 0x%02x -> V%zu\n", kk, x);
					// print_registers();
					break;
				case 0x8000: // 8xy4 ADD Vx, Vy
					this->v[y] += this->v[x]; 
					if (VERBOSE_CPU) printf("V%i V%i\n", this->v[x], this->v[y]);
					break;
				case 0x001E: // Fx1E ADD I = I + Vx
					this->i += this->v[x];
					if (VERBOSE_CPU) printf("I, V%zu\n", x);
					break;
			}
			break;
		case Op::CALL: // Call subroutine
			this->stack.push(this->pc);
			this->pc = nnn; // Store address into program counter
			this->pc -= 2;
			break;
		case Op::CLS: // Clear screen
			if (VERBOSE_CPU) printf("%s\n", opstr);
			// Clear 64x32 display
			for(int i = 0; i < DISP_X*DISP_Y; i++)
				chip8->gfx[i] = 0;
			chip8->draw_flag = true;
			break;
		case Op::JP: 
			switch(opcode & 0xF000){
				default:
					if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04x}\n", opcode);
					break;
				case 0x1000: // 1nnn - jump to address nnn
					this->pc = nnn;
					if (VERBOSE_CPU) printf("0x%03x\n", nnn);
					this->pc -= 2;
					break;
				case 0xB000: // Bnnn - jump to address nnn + v[0]
					this->pc = nnn + v[0];
					if (VERBOSE_CPU) printf("0x%03x + 0x%03x\n", v[0], nnn);
					this->pc -= 2;
					break;
			}
			break;
		case Op::DRW:
		{
			if (VERBOSE_CPU) printf("Vx: 0x%01x Vy: 0x%01x n: %i i: 0x%02x\n", this->v[x], this->v[y], n, this->i);
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
						if(chip8->gfx[(this->v[x] + dx + ((this->v[y] + dy) * DISP_X))]){
							// Set VF flag to 1 indicating that at least one pixel was unset
							this->v[0xF] = 1;
						}
						chip8->gfx[this->v[x] + dx + ((this->v[y] + dy) * DISP_X)] ^= 1;
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
					if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04x}\n", opcode);
					break;
				case 0x6000: // 6xkk - Set Vx to kk
					this->v[x] = kk;
					if (VERBOSE_CPU) printf("V%zu 0x%02x\n", x, kk);
					this->pc -= 2; // TODO: I don't know how this is right, but I fucking want to know why
					break;
				case 0x8000: // 8xy0 - Set Vx to Vy
					this->v[x] = this->v[y];
					if (VERBOSE_CPU) printf("V%i V%i\n", this->v[x], this->v[y]);
					break;
				case 0xA000: // annn - Set i to address nnn
					this->i = nnn;
					if (VERBOSE_CPU) printf("0x%03x -> I\n", nnn);
					break;
				case 0xF000:
					switch(opcode & 0x00FF){
						default:
							if (VERBOSE_CPU) printf("\nError: Invalid opcode {%04x}\n", opcode);
							break;
						case 0x0007: // Fx07 - LD Vx, DT "load Vx into DT"
							this->dt = this->v[x];
							if (VERBOSE_CPU) printf("V%zu DT\n", x);
							break;
						case 0x000A: // Fx0A - LD Vx, K
						{
							// TODO:
							// Wait for keypress, then store value of key in Vx
							InputHandler::PollKeyUntilEvent();
							//
							if (G_last_key_pressed < 0x10)
								this->v[x] = InputHandler::GetKeyRegister(G_last_key_pressed);
							else
								if (VERBOSE_CPU) printf("Last key pressed was not mapped\n");
							if (VERBOSE_CPU) printf("Vx, k NOT IMPLEMENTED\n");
							break;
						}
						case 0x0015: // Fx15 - LD DT, Vx
							this->v[x] = this->dt;
							if (VERBOSE_CPU) printf("V%zu\n", x);
							break;
						case 0x0018: // Fx18 - LD ST, Vx
							this->v[x] = this->st;
							if (VERBOSE_CPU) printf("V%zu\n", x);
							break;
						case 0x0029: // Fx29 - LD F Vx -> I
									 // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. 
							if (VERBOSE_CPU) printf("F V%zu -> I\n", x);
							this->i = this->v[x] * 0x5; // Each font is 5 bytes wide (as shown in textfont) 
							break;
						case 0x0033: // Fx33 - LD B, Vx
									 // Store BCD representation of Vx in mem locations I, I+1, and I+2.
									 // BCD = Binary coded representation, see https://www.techtarget.com/whatis/definition/binary-coded-decimal
							if (VERBOSE_CPU) printf("LD B, V%zu\n", x);
							this->mem[this->i] = this->v[x] / 100; // Load 100s place into memory
							this->mem[this->i+1] = (this->v[x] / 10) % 10; // Load 10s place into memory
							this->mem[this->i+2] = this->v[x] % 10; // Load 1s place into memory
							break;
						case 0x0055: // Fx55 - LD [I], Vx
							for (uint8_t i = 0; i <= x; i++){
								// Stores from V0 to VX (including VX) into memory, starting at address I. The offset from I is increased by 1 for each value written, 
								// but I itself is left unmodified.
								if (VERBOSE_CPU) printf("I -> V%zu\n", x);
								this->mem[this->i + i] = this->v[i];
							}
							this->i += x + 1;
							break;
						case 0x0065: // Fx65 - LD Vx, [I]
									 // Read from memory starting at address I into v registers
							if (VERBOSE_CPU) printf("LD V0-V%zu -> I\n", x);
							for (uint8_t i = 0; i <= x; i++){
								this->v[i] = this->mem[this->i + i];
							}

							this->i += x + 1;
							break;	
					}
			}
		}
		case Op::SE:
			// The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
			if (this->v[x] == kk){ 
				if (VERBOSE_CPU) printf("SKIPPING\n");
				this->pc += 2;
				if (VERBOSE_CPU) printf("V%zu: 0x%02x == 0x%02x\n", x, this->v[x], kk);
			} else {
				if (VERBOSE_CPU) printf("NOT SKIPPING\n");
				if (VERBOSE_CPU) printf("V%zu: 0x%02x != 0x%02x\n", x, this->v[x], kk);
			}
			break;
		case Op::SNE:
		{
			// Skip next instruction if Vx != kk
			if (this->v[x] != kk){
				if (VERBOSE_CPU) printf("SKIPPING\n");
				this->pc += 2;
				if (VERBOSE_CPU) printf("V%zu: 0x%02x != 0x%02x\n", x, this->v[x], kk);
			} else {
				if (VERBOSE_CPU) printf("NOT SKIPPING\n");
				if (VERBOSE_CPU) printf("V%zu: 0x%02x == 0x%02x\n", x, this->v[x], kk);
			}
			// if (VERBOSE_CPU) printf("WARNING: opcode not implemented yet\n");

			break;
		}
		case Op::SKP: // Ex9E - SKP Vx 
		{
			// TODO:
			// Skip next instruction if key with value of Vx is pressed
			G_last_key_pressed = InputHandler::PollKeyFor(1); // "Poll key for 1 tick"

			if (this->v[x] == InputHandler::GetKeyRegister(G_last_key_pressed)){
				if (VERBOSE_CPU) printf("SKIPPING, Vx == K\n");
				this->pc += 2;
			} else {
				if (VERBOSE_CPU) printf("NOT SKIPPING, Vx != K\n");
			}
			// if (VERBOSE_CPU) printf("Warning: op is not implemented yet!\n");
			break;
		}
		case Op::SKNP:
		{
			// TODO:
			G_last_key_pressed = InputHandler::PollKeyFor(1);
			if (VERBOSE_CPU) printf("Last key: 0x%01x\n", G_last_key_pressed);
			if (this->v[x] != InputHandler::GetKeyRegister(G_last_key_pressed)){
				if (VERBOSE_CPU) printf("SKIPPING, Vx == K\n");
				this->pc += 2;
			} else {
				if (VERBOSE_CPU) printf("NOT SKIPPING, Vx != K\n");
			}
			// if (VERBOSE_CPU) printf("Warning: op is not implemented yet!\n");
			break;
		}
		case Op::RET:
			if (VERBOSE_CPU) printf("top: 0x%04x\n", stack.top());
			this->pc = this->stack.top();
			this->stack.pop();
			break;
		case Op::RND: // RND Vx 
			this->v[x] = (rand() % 0xFF) & 0xFF; // Set Vx to random # from (0-255), then & 255
			if (VERBOSE_CPU) printf("RND V%zu = 0x%02x\n", x, v[x]);
			break;
		case Op::SYS: // Ignored
			if (VERBOSE_CPU) printf("\n");
			break;
		case Op::XOR: // Exclusive OR
			// Performs a bitwise XOR on the values of Vx and Vy, then stores the result in Vx.
			if (VERBOSE_CPU) printf("V%zu = Vx ^ Vy = 0x%02x\n", x, this->v[x] ^ this->v[y]);
			this->v[x] ^= this->v[y];
			break;
		case Op::ERR:
			if (VERBOSE_CPU) printf("You fucked up kid\n");
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
	// printf("sp: 0x%04x\n", this->sp);
	printf("i: 0x%04x\n", this->i);
	printf("pc: 0x%04x\n", this->pc);
	printf("dt: 0x%02x\n", this->dt);
	printf("st: 0x%02x\n", this->st);
	printf("------------\n");
}

void CPU::print_args(uint16_t opcode){
	printf("opcode: 0x%04x\n", opcode);
	printf("op: %s\n", Op::optostr(decode(opcode)));
	size_t x = Op::x(opcode);
	size_t y = Op::y(opcode);
	uint8_t kk = Op::kk(opcode);
	uint16_t nnn = Op::nnn(opcode);
	uint8_t n = Op::n(opcode);
	printf("x: %zu\n", x);
	printf("y: %zu\n", y);
	printf("kk: 0x%02x\n", kk);
	printf("nnn: 0x%03x\n", nnn);
	printf("n: 0x%01x\n", n);
}

