#define DEBUG_MODE false

#include <chip8.h>
#include <cpu.h>
#include <display.h>
#include <input.h>
#include <clock.h>

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

SDL_Window* window = SDL_CreateWindow("CHIP8", 
									SDL_WINDOWPOS_CENTERED, 
									SDL_WINDOWPOS_CENTERED, 
									SCREEN_X, 
									SCREEN_Y, 
									0
								  );

int main(int argc, char *argv[]){
	const char* rom_path = argv[1];
	if (argc != 2){
		printf("Error: Need to enter the path to the file as the argument\n");
		return 0;
	}
	// SDL Rendering stuff
	SDL_Renderer* renderer = NULL;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	if (SDL_Init(SDL_INIT_EVERYTHING)){
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 0;
	}

	// Chip8 initialization & cycles
	printf("===============START================\n");
	Clock clock;
	Chip8 chip8(rom_path); 
	CPU cpu(&chip8, &clock); 
	Display disp(&chip8);
	
	bool quit = false;
	size_t num_pixels = 0;	
	// size_t num_cycles = 50;
	// for (int i = 0; i < num_cycles; i++){
	size_t cycles = 0;
	while(!quit){
		// InputHandler::PollKey(&chip8);
		cycles++;
		cpu.cycle();
		disp.RenderGFX(&num_pixels, renderer);
		if (DEBUG_MODE){
			printf("Cycles: %zu\n", cycles);
			// cpu.print_registers();
			// frame by frame execution 
			getchar(); 
		}
		// Count down dt if it is non-zero
		cpu.delay_timer();
		clock.tick();
	}

    SDL_DestroyWindow(window);
    SDL_Quit();
	
	return 1;
} 

