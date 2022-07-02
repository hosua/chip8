#include <chip8.h>
#include <cpu.h>
#include <display.h>
#include <input.h>
#include <clock.h>

// For arg parser
#include <unistd.h>

SDL_Window* window = SDL_CreateWindow("CHIP8", 
									SDL_WINDOWPOS_CENTERED, 
									SDL_WINDOWPOS_CENTERED, 
									SCREEN_X, 
									SCREEN_Y, 
									0
								  );

int main(int argc, char *argv[]){
	// The cycle at which the emulator will start on (to make debugging less of a chore)
	size_t start_cycle = 0;
	int o;
	std::string rom_str;
	while ((o = getopt(argc, argv, ":d:")) != -1){
		switch (o){
			// Debug mode
			case 'd':
				if (optarg)
					start_cycle = std::atoi(optarg);
				printf("Running chip8 in debug mode...\n");
				DEBUG_MODE = true;
				break;
			default:
				printf("Error: When using debug mode, you must specify the frame to start on as an argument.\n");
				exit(1);
				break;
		}
	} 
	// Parse non-option args
	for (int i = optind; i < argc; i++)
		rom_str += argv[i];	
	


	Chip8 chip8;
	const char* rom_path = rom_str.c_str();
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
	chip8.LoadROM(rom_path);
	CPU cpu(&chip8, &clock);
	Display disp(&chip8);
	
	bool quit = false;
	size_t num_pixels = 0;	

	size_t cycles = 0;
	while(cycles < start_cycle-1){
		InputHandler::GetChip8Keys(&chip8);
		cpu.cycle();
		disp.RenderGFX(&num_pixels, renderer);
		printf("Cycles: %zu\n", cycles);
		// Count down dt if it is non-zero
		cpu.delay_timer();
		clock.tick();
		cycles++;
	}

	while(!quit){
		InputHandler::GetChip8Keys(&chip8);
		cycles++;
		cpu.cycle();
		disp.RenderGFX(&num_pixels, renderer);
		if (DEBUG_MODE){
			printf("Cycles: %zu\n", cycles);
			// cpu.print_registers();
			// Execute each frame only when pressing a valid chip8 key
			InputHandler::WaitForKeyPress();
		}
		// Count down dt if it is non-zero
		cpu.delay_timer();
		clock.tick();
	}

    SDL_DestroyWindow(window);
    SDL_Quit();
	
	return 1;
} 

