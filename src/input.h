#ifndef INPUT_H
#define INPUT_H
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <map>

#include <chip8.h>


extern uint8_t G_last_key_pressed;

namespace InputHandler {
	void PrintKeyInfo(SDL_KeyboardEvent *key);
	uint8_t GetKeyRegister(uint8_t scancode);
	void PollKey(uint8_t* last_key_pressed);
	uint8_t PollKeyFor(uint16_t num_ticks);
	void PollKeyUntilEvent();
}

#endif
