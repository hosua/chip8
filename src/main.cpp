#include <chip8.h>
#include <cpu.h>
#include <display.h>
#include <input.h>
#include <clock.h>
#include <dir_nav.h>
#include <iostream>
#include <filesystem>


// For parsing CLI args
#include <getopt.h>

bool SLOW_MODE = false;
bool DEBUG_MODE  = false;
bool VERBOSE_CLOCK = false;
bool VERBOSE_CPU = false;
bool VERBOSE_DISPLAY = false;
bool VERBOSE_INPUT = false;

void help_menu(){
	printf("Options:\n"
			"-d, --debug-mode <start_frame>\tEnable step-by-step execution and skip to the specified frame\n"
			"-v, --verbose <type>\t\tTypes: cpu clock display input (Can only take one parameter)\n"
			"-s, --slow-mode\t\t\tRuns the emulator at a slower speed\n"
			"-h, --help\t\t\tThis help menu\n");
}

SDL_Window* window;

int main(int argc, char *argv[]){
	// The cycle at which the emulator will start on (to make debugging less of a hassle)
	size_t start_frame = 0;
	int o;
	int opt_index = 0;

	const struct option long_opts[] =
	{
		{"path", required_argument, 0, 'p'},
		{"debug-mode", 	  optional_argument,  0, 'd'},
		{"verbose",   optional_argument,  0, 'v'},
		{"slow-mode",   no_argument,  0, 's'},
		{"help",   no_argument,  0, 'h'},
		{0,0,0,0},
	};

	std::string rom_str;

	while ((o = getopt_long(argc, argv, "hsp:v::d::", long_opts, &opt_index)) != -1){
		switch (o){
			// Debug mode
			case 'd':
				// Get optarg if exists
				if (optarg == NULL && optind < argc
						&& argv[optind][0] != '-')
					optarg = argv[optind++];

				if (optarg) start_frame = std::atoi(optarg);
				printf("Running chip8 in debug mode...\n");
				DEBUG_MODE = true;
				break;
			case 'v':
				{
					// Multiple args for one flag is not possible
					if (optarg == NULL && optind < argc
							&& argv[optind][0] != '-')
						optarg = argv[optind++];

					if (optarg == NULL){
						VERBOSE_CPU = true;
						VERBOSE_CLOCK = true;
						VERBOSE_DISPLAY = true;
						VERBOSE_INPUT = true;
					} else {
						if (strcmp(optarg, "cpu") == 0)
							VERBOSE_CPU = true;
						if (strcmp(optarg, "clock") == 0)
							VERBOSE_CLOCK = true;
						if (strcmp(optarg, "display") == 0)
							VERBOSE_DISPLAY = true;
						if (strcmp(optarg, "input") == 0)
							VERBOSE_INPUT = true;
					}
				}
				break;
			case 's':
				SLOW_MODE = true;
				break;
			case 'h':
				help_menu();
				exit(0);
				break;
			case 'p':
				// If path flag is passed, use its argument as the rom string
				rom_str = SelectGame(optarg).c_str();
				break;
			case '?':
				printf("Error parsing arguments.\n");
				break;
			default:
				help_menu();
				exit(1);
				break;
		}
	}

	if (rom_str == "")
		rom_str = SelectGame(DEFAULT_GAMES_DIR).c_str();
	

	Chip8 chip8;
	const char* rom_path = rom_str.c_str();
	std::cout << std::string(rom_path) << std::endl;
	// SDL Rendering stuff
	SDL_Renderer* renderer = NULL;
	window = SDL_CreateWindow("CHIP8", 
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 
			SCREEN_X, 
			SCREEN_Y, 
			SDL_WINDOW_RESIZABLE
		);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	// Resolution-independent scaling
	SDL_RenderSetLogicalSize(renderer, SCREEN_X, SCREEN_Y);
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
	// Each cycle is equivalent to one frame
	size_t cycles = 0;
	// If running in debug mode, this while loop will bring us to the specified frame
	if (start_frame){
		printf("Jumping to frame %zu...\n", start_frame);
		while(cycles < start_frame-1){
			InputHandler::GetChip8Keys(&chip8);
			cpu.cycle();
			disp.RenderGFX(&num_pixels, renderer);
			if (DEBUG_MODE)
				printf("Cycles: %zu\n", cycles);

			cpu.delay_timer();
			clock.tick();
			cycles++;
		}
		printf("Finished jumping to frame %zu.\n", start_frame);
	}

	while(!quit){
		InputHandler::GetChip8Keys(&chip8);
		cycles++;
		cpu.cycle();
		disp.RenderGFX(&num_pixels, renderer);
		if (DEBUG_MODE){
			printf("Cycles: %zu\n", cycles);
			// Execute each cycle only when pressing a valid chip8 key
			InputHandler::WaitForKeyPress();
		}
		cpu.delay_timer();
		clock.tick();
		if (VERBOSE_INPUT) InputHandler::PrintChip8Keys(&chip8);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 1;
} 

