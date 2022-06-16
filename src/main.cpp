#include <chip8.h>
#include <cpu.h>
#include <display.h>

int main(int argc, char *argv[]){
	const char* rom_path = argv[1];
	if (argc != 2){
		printf("Error: Need to enter the path to the file as the argument\n");
		return 0;
	}
	printf("===============START================\n");
	Chip8 chip8;
	chip8.LoadROM(rom_path); // ROM must load before CPU is initialized
	CPU cpu(&chip8);
	Display disp(&chip8);
	chip8.print_display();
	// size_t cycles = 50;
	// for (int i = 0; i < cycles; i++){
	for (;;){
		cpu.cycle();
		disp.DrawScreen();
		getchar();
	}
	return 1;
} 

