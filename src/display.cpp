#include <chip8.h>
#include <display.h>
#include <cpu.h>

#define DEBUG_MODE true

const char* PX = "\u2588\u2588";

SDL_Rect Display::GetPixel(uint8_t x, uint8_t y){
	// Create a 10x10 rectangle (the pixel)
    SDL_Rect pixel;
    pixel.x = x * PIXEL_SIZE;
    pixel.y = y * PIXEL_SIZE;

    pixel.w = PIXEL_SIZE;
    pixel.h = PIXEL_SIZE;
	return pixel;
}


void Display::RenderGFX(size_t* num_pixels, SDL_Renderer *renderer){
	uint16_t x_pos = 1;
	uint16_t y_pos = 1;
	std::vector<SDL_Rect> set_vect;
	std::vector<SDL_Rect> unset_vect;
	if (chip8->draw_flag){
		chip8->draw_flag = false;
		for (int i = 0; i < DISP_X*DISP_Y; i++, x_pos++){
			if (chip8->gfx[i])
				set_vect.push_back(GetPixel(x_pos, y_pos));
			else
				unset_vect.push_back(GetPixel(x_pos, y_pos));

			if (((i+1) % DISP_X) == 0){
				y_pos++;
				x_pos = 0;
			}
		}

		// Display graphics into terminal
		if (DEBUG_MODE){
			for (int i = 0; i < DISP_X*DISP_Y; i++){
				if (chip8->gfx[i]) {
					printf("%s", PX);
				} else {
					printf("  ");
				}
				if (((i+1) % DISP_X) == 0) {
					printf("\n");
				}
			}
		}
	}

	SDL_Rect set_arr[set_vect.size()];
	std::copy(set_vect.begin(), set_vect.end(), set_arr);

	SDL_Rect unset_arr[unset_vect.size()];
	std::copy(unset_vect.begin(), unset_vect.end(), unset_arr);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	RenderPixels(renderer, set_arr, set_vect.size());

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	RenderPixels(renderer, unset_arr, unset_vect.size());
}

void Display::RenderPixels(SDL_Renderer* renderer, SDL_Rect* pixel_arr, size_t num_pixels){
	SDL_RenderFillRects(renderer, pixel_arr, num_pixels);
    SDL_RenderPresent(renderer);
}


