#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include <chip8.h>
#include <cpu.h>

#define SCREEN_X 1320
#define SCREEN_Y 680

#define PIXEL_SIZE 20

// A black square to draw on the terminal
extern const char* PX;

class Display {
public:
	Chip8* chip8;
	Display(Chip8* chip8){
		this->chip8 = chip8;
	}
  
	void GetGfx();
	void DrawPixel(uint16_t x, uint16_t y, SDL_Renderer *renderer, SDL_Window *window);
	void ClearPixel(uint16_t x, uint16_t y, SDL_Renderer *renderer, SDL_Window *window);

	void DrawScreen(SDL_Renderer* renderer, SDL_Window* window);
};

#endif
