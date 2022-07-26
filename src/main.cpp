#include <chip8.h>
#include <cpu.h>
#include <display.h>
#include <input.h>
#include <clock.h>

// For parsing CLI args
#include <getopt.h>

bool SLOW_MODE = false;
bool DEBUG_MODE  = false;
bool VERBOSE_CLOCK = false;
bool VERBOSE_CPU = false;
bool VERBOSE_DISPLAY = false;
bool VERBOSE_INPUT = false;

SDL_Window* window = SDL_CreateWindow("CHIP8", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		SCREEN_X, 
		SCREEN_Y, 
		0
		);

void help_menu(){
	printf("\nTo run a game, pass the path to the ROM as an argument e.g. ./CHIP8 \"path/to/rom\"\n"
			"Options:\n"
			"-d, --debug-mode <start_frame>\tEnable step-by-step execution and skip to the specified frame\n"
			"-v, --verbose <type>\t\tTypes: cpu, clock, display, input\n"
			"-s, --slow-mode\t\t\tRuns the emulator at a slower speed\n"
			"-h, --help\t\t\tThis help menu\n");
}

int main(int argc, char *argv[]){
	// The cycle at which the emulator will start on (to make debugging less of a chore)
	size_t start_frame = 0;
	int o;
	std::string rom_str;
	int opt_index = 0;

	const struct option long_opts[] =
	{
		{"debug-mode", 	  optional_argument,  0, 'd'},
		{"verbose",   required_argument,  0, 'v'},
		{"slow-mode",   no_argument,  0, 's'},
		{"help",   no_argument,  0, 'h'},
		{0,0,0,0},
	};
	while ((o = getopt_long(argc, argv, "hsv:d::", long_opts, &opt_index)) != -1){
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
					// Parse multiple args for v
					for (int i = optind - 1; i < argc; i++){
						// If the argument is a flag, break
						if (argv[i][0] == '-')
							break;
						else {
							const char* arg = argv[i];
							if (strcmp(arg, "cpu") == 0)
								VERBOSE_CPU = true;
							if (strcmp(arg, "clock") == 0)
								VERBOSE_CLOCK = true;
							if (strcmp(arg, "display") == 0)
								VERBOSE_DISPLAY = true;
							if (strcmp(arg, "input") == 0)
								VERBOSE_INPUT = true;
						}
					}
					break;
				}
			case 's':
				SLOW_MODE = true;
				break;
			case 'h':
				help_menu();
				exit(0);
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
	// Parse non-option args (This really should only ever be one argument, the path to the ROM.)
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

