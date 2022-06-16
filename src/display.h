#ifndef DISPLAY_H
#define DISPLAY_H

#include <chip8.h>
#include <cpu.h>

// A black square to draw on the terminal
extern const char* PX;

class Display {
public:
	Chip8* chip8;
	Display(Chip8* chip8){
		this->chip8 = chip8;
	}

	void DrawScreen();
};

#endif
