#include <input.h>
#include <display.h>



namespace Input {
	// a global variable to store the last key pressed
	uint8_t last_key = 0;
	// key_map<scancode, register>
	std::map<uint8_t, uint8_t> key_map = {
		{0x1E, 0x1}, {0x1F, 0x2}, {0x20, 0x3}, {0x21, 0xC},
		{0x14, 0x4}, {0x1A, 0x5}, {0x08, 0x6}, {0x15, 0xD},
		{0x04, 0x7}, {0x16, 0x8}, {0x07, 0x9}, {0x09, 0xE},
		{0x1D, 0xA}, {0x1B, 0x0}, {0x06, 0xB}, {0x19, 0xF},
	};

	/* Print all information about a key event */
	void PrintKeyInfo(SDL_KeyboardEvent *key){
		/* Is it a release or a press? */
		if(key->type == SDL_KEYUP)
			printf("Released ");
		else
			printf("Pressed ");

		/* Print the hardware scancode first */
		printf("Scancode: 0x%02X", key->keysym.scancode);
		/* Print the name of the key */
		printf(" %s", SDL_GetKeyName( key->keysym.sym));
		/* We want to print the unicode info, but we need to make */
		/* sure its a press event first (remember, release events */
		/* don't have unicode info                                */
		if(key->type == SDL_KEYDOWN){
			/* If the Unicode value is less than 0x80 then the    */
			/* unicode value can be used to get a printable       */
			/* representation of the key, using (char)unicode.    */
		   printf("(0x%04X)", key->keysym.sym);
		}
		printf("\n");
	}
	// Convert SDL scancode into v register index x
	// If this returns 0x10, then an invalid key was pressed
	uint8_t GetKeyRegister(uint8_t scancode){
		uint8_t v_reg = 0x10;
		std::map<uint8_t, uint8_t>::iterator itr;	
		itr = key_map.find(scancode);
		if (itr != key_map.end()){
			v_reg = key_map[scancode];
			printf("v_reg: 0x%01x ", v_reg);
		}
		return v_reg;
	}

	// Polls continuously in main and returns scancode of the key pressed
	void PollKey(){
		SDL_Event event;
		SDL_PollEvent(&event);
		SDL_KeyboardEvent *key = &event.key;
		uint8_t scancode = key->keysym.scancode;
		switch (event.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				if (Input::key_map.find(scancode) != key_map.end()){
					Input::PrintKeyInfo(key);
					// Store last key pressed in global
					Input::last_key = GetKeyRegister(scancode);
				}
				if (scancode == SDL_SCANCODE_ESCAPE){
					printf("Exiting... Goodbye!\n");
					SDL_DestroyWindow(window);
					SDL_Quit();
					exit(1);
				}
				break;
			}
			case SDL_QUIT:
			{
				break;
			}
		}
	}
}
