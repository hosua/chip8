#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <map>

namespace Input {
	extern uint8_t last_key;
	extern std::map<uint8_t, uint8_t> key_map;

	void PrintKeyInfo(SDL_KeyboardEvent *key);
	uint8_t GetKeyRegister(uint8_t scancode);
	uint8_t PollKey(SDL_Window* window);
}
