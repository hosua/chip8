
#include <chip8.h>
#include <cpu.h>
#include <display.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

int main(int argc, char *argv[]){
	const char* rom_path = argv[1];
	if (argc != 2){
		printf("Error: Need to enter the path to the file as the argument\n");
		return 0;
	}

	// SDL Rendering stuff
	SDL_Renderer* renderer = NULL;
	SDL_Window* window = SDL_CreateWindow("CHIP8", 
										SDL_WINDOWPOS_CENTERED, 
										SDL_WINDOWPOS_CENTERED, 
										SCREEN_X, 
										SCREEN_Y, 
										0
									  );

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	if (SDL_Init(SDL_INIT_EVERYTHING)){
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 0;
	}

	// Chip8 initialization & cycles
	printf("===============START================\n");
	Chip8 chip8;
	chip8.LoadROM(rom_path); // ROM must load before CPU is initialized
	CPU cpu(&chip8);
	Display disp(&chip8);
	chip8.print_display();


	// size_t num_cycles = 50;
	// for (int i = 0; i < num_cycles; i++){
	size_t cycles = 0;
	for (;;){
		cycles++;
		printf("Cycles: %zu\n", cycles);
		cpu.cycle();
		disp.DrawScreen(renderer, window);
		getchar(); // frame by frame execution (comment/uncomment this for debugging)
		
	}

    // Wait for 5 sec
    SDL_Delay(5000);

    SDL_DestroyWindow(window);
    SDL_Quit();
	
	return 1;
} 

