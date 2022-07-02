#include <input.h>
#include <display.h>
// key_map<scancode, register>
std::map<uint8_t, uint8_t> key_map = {
	{0x1E, 0x1}, {0x1F, 0x2}, {0x20, 0x3}, {0x21, 0xC},
	{0x14, 0x4}, {0x1A, 0x5}, {0x08, 0x6}, {0x15, 0xD},
	{0x04, 0x7}, {0x16, 0x8}, {0x07, 0x9}, {0x09, 0xE},
	{0x1D, 0xA}, {0x1B, 0x0}, {0x06, 0xB}, {0x19, 0xF},
};

// For debugging
void InputHandler::PrintChip8Keys(Chip8* chip8){
	bool* keys = chip8->keys;
	// 1 2 3 C
	printf("KEYBOARD:\n");
	for (int i = 0x1; i <= 0x3; i++) 
		printf("%i ", keys[i]);
	printf("%i\n", keys[0xC]);
	// 4 5 6 D
	for (int i = 0x4; i <= 0x6; i++) 
		printf("%i ", keys[i]);
	printf("%i\n", keys[0xD]);
	// 7 8 9 E
	for (int i = 0x7; i <= 0x9; i++) 
		printf("%i ", keys[i]);
	printf("%i\n", keys[0xE]);
	// A 0 B F
	printf("%i %i %i %i\n", keys[0xA], keys[0x0], keys[0xB], keys[0xF]);
	printf("\n");
}

/* Print all information about a key event */
void InputHandler::PrintKeyInfo(SDL_KeyboardEvent *key){
	/* Is it a release or a press? */
	if(key->type == SDL_KEYUP)
		printf("Released ");
	else
		printf("Pressed ");
	printf("Scancode: 0x%02X", key->keysym.scancode);
	printf(" %s", SDL_GetKeyName( key->keysym.sym));
	if(key->type == SDL_KEYDOWN){
	   printf("(0x%04X)", key->keysym.sym);
	}
	printf("\n");
}


// Gets and sets the flags in the chip8 keys array which determine if they are being pressed or not.
void InputHandler::GetChip8Keys(Chip8* chip8){
	/* "One important thing to know about how SDL handles key states is that you still need an event loop running. 
	 * SDL's internal keystates are updated every time SDL_PollEvent is called, so make sure you polled all 
	 * events on queue before checking key states." https://lazyfoo.net/tutorials/SDL/18_key_states/index.php */
	SDL_Event event;
	SDL_PollEvent(&event);
	const uint8_t *kb_state = SDL_GetKeyboardState(NULL);
	if (kb_state[SDL_SCANCODE_ESCAPE]){
		ExitChip8();
	}
	for (std::map<uint8_t, uint8_t>::iterator itr = key_map.begin(); itr != key_map.end(); itr++){
		uint8_t key_code = itr->first;
		uint8_t key_reg = itr->second;
		if (kb_state[key_code])
			chip8->keys[key_reg] = true;		
		else
			chip8->keys[key_reg] = false;
	}
}

// Waits for a valid Chip8 key to be pressed and returns its scancode
uint8_t InputHandler::WaitForKeyPress(){
	SDL_Event event;
	SDL_KeyboardEvent *key;
	uint8_t scancode;
	uint32_t event_type;
	// Wait until a key event before continuing code execution
	while(SDL_WaitEvent(&event)){
		key = &event.key;
		event_type = key->type;
		scancode = key->keysym.scancode;

		if (scancode == SDL_SCANCODE_ESCAPE)
			if (!DEBUG_MODE) ExitChip8();

		// Only break if we find a valid key mapped to the chip8
		if (event_type == SDL_KEYUP && (key_map.find(scancode) != key_map.end())) 
			break;
	}
	return scancode;
}

// Convert SDL scancode into v register index x
// If this returns 0x10, then an invalid or no key was pressed
uint8_t InputHandler::GetKeyRegister(uint8_t scancode){
	uint8_t v_reg = 0x10;

	if (key_map.find(scancode) != key_map.end()){
		v_reg = key_map[scancode];
		printf("v_reg: 0x%01x\n", v_reg);
	}
	return v_reg;
}

