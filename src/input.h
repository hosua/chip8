#ifndef INPUT_H
#define INPUT_H
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <map>

#include <chip8.h>

namespace InputHandler {
	// For debugging
	void PrintChip8Keys(Chip8* chip8); 
	void PrintKeyInfo(SDL_KeyboardEvent *key); 
	// Gets and sets the flags in the chip8 keys array which determine if they are being pressed or not.
	void GetChip8Keys(Chip8* chip8);
	// Stops all code execution and waits for a valid chip8 key to be pressed, resumes execution, and 
	// then returns the scancode value of the key pressed.
	uint8_t WaitForKeyPress();
	// Takes a key's scancode as the parameter and returns the v register index from our global key_map
	uint8_t GetKeyRegister(uint8_t scancode);
}

#endif
