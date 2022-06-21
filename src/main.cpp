
#include <chip8.h>
#include <cpu.h>
#include <display.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

// #define DEBUG_MODE true
#define DEBUG_MODE false

/* Print all information about a key event */
void PrintKeyInfo( SDL_KeyboardEvent *key ){
	/* Is it a release or a press? */
	if( key->type == SDL_KEYUP )
		printf( "Released " );
	else
		printf( "Pressed " );

	/* Print the hardware scancode first */
	printf( "Scancode: 0x%02X", key->keysym.scancode );
	/* Print the name of the key */
	printf( " %s", SDL_GetKeyName( key->keysym.sym ) );
	/* We want to print the unicode info, but we need to make */
	/* sure its a press event first (remember, release events */
	/* don't have unicode info                                */
	if( key->type == SDL_KEYDOWN ){
		/* If the Unicode value is less than 0x80 then the    */
		/* unicode value can be used to get a printable       */
		/* representation of the key, using (char)unicode.    */
	   printf( "(0x%04X)", key->keysym.sym );
	}
	printf( "\n" );
	/* Print modifier info */
}

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
	
	SDL_Event event;
	bool quit = false;
	
	// size_t num_cycles = 50;
	// for (int i = 0; i < num_cycles; i++){
	size_t cycles = 0;
	while(!quit){
		SDL_PollEvent(&event);
		switch (event.type){
		case SDL_KEYDOWN:
		{
			PrintKeyInfo(&event.key);
			break;
		}
		case SDL_QUIT:
		{
			quit = true;
			break;
		}
		}
		cycles++;
		// printf("Cycles: %zu\n", cycles);
		cpu.cycle();
		disp.DrawScreen(renderer);
		if (DEBUG_MODE){
			cpu.print_registers();
			// frame by frame execution 
			getchar(); 
		}
		
	}

    // Wait for 5 sec
    SDL_Delay(5000);

    SDL_DestroyWindow(window);
    SDL_Quit();
	
	return 1;
} 

