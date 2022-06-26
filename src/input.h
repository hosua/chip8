#ifndef INPUT_H
#define INPUT_H
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <map>

#include <chip8.h>

namespace InputHandler {
	void PrintChip8Keys(Chip8* chip8);
	void PrintKeyInfo(SDL_KeyboardEvent *key);
	void GetChip8Keys(Chip8* chip8);
	uint8_t WaitForKeyPress();
	uint8_t GetKeyRegister(uint8_t scancode);
	void PollKey();
}

#endif
