#include <chip8.h>
#include <display.h>
#include <cpu.h>


SDL_Rect Display::GetSetPixel(uint8_t x, uint8_t y, SDL_Renderer *renderer){
	// Set render color to white ( rect will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
	// Create a 10x10 rectangle (the pixel)
    SDL_Rect pixel;
    pixel.x = x * PIXEL_SIZE;
    pixel.y = y * PIXEL_SIZE;

    pixel.w = PIXEL_SIZE;
    pixel.h = PIXEL_SIZE;
	return pixel;
}

SDL_Rect Display::GetUnsetPixel(uint8_t x, uint8_t y, SDL_Renderer *renderer){
	// Set render color to white ( rect will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
	// Create a 10x10 rectangle (the pixel)
	
    SDL_Rect pixel;
    pixel.x = x * PIXEL_SIZE;
    pixel.y = y * PIXEL_SIZE;

    pixel.w = PIXEL_SIZE;
    pixel.h = PIXEL_SIZE;
	return pixel;
}

SDL_Rect* Display::SetPixels(SDL_Renderer *renderer){
	uint16_t x_pos = 1;
	uint16_t y_pos = 1;
	std::vector<SDL_Rect> pixel_vect;
	if (chip8->draw_flag){
		chip8->draw_flag = false;
		for (int i = 0; i < DISP_X*DISP_Y; i++){
			if (chip8->gfx[i]){
				pixel_vect.push_back(GetSetPixel(x_pos, y_pos, renderer));
			} else {
				// Instead of clearing a pixel every time, we're using screen as a buffer to check
				// if we actually have to unset it. Drawing unnecessary rectangles is computationally expensive.
				if (this->screen[i]){
					pixel_vect.push_back(GetUnsetPixel(x_pos, y_pos, renderer));
				}
			}
			if (((i+1) % DISP_X) == 0){
				y_pos++;
				x_pos = 0;
			}
			x_pos++;
		}

		// Display graphics into terminal
		/*
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
		*/
	}
	GetScreen();
}
// Draw pixel into renderer and return the rect
void Display::DrawPixel(uint8_t x, uint8_t y, SDL_Renderer *renderer){
	// Set render color to white ( rect will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
	std::vector<SDL_Rect> rect_vect;
	// Create a 10x10 rectangle (the pixel)
    SDL_Rect pixel;
    pixel.x = x * PIXEL_SIZE;
    pixel.y = y * PIXEL_SIZE;

    pixel.w = PIXEL_SIZE;
    pixel.h = PIXEL_SIZE;

    // Render rect
    SDL_RenderFillRect( renderer, &pixel );
    // Render the rect to the screen
    SDL_RenderPresent(renderer);
}

void Display::ClearPixel(uint8_t x, uint8_t y, SDL_Renderer *renderer){
	// Set render color to white ( rect will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
	// Create a 10x10 rectangle (the pixel)
	
    SDL_Rect pixel;
    pixel.x = x * PIXEL_SIZE;
    pixel.y = y * PIXEL_SIZE;

    pixel.w = PIXEL_SIZE;
    pixel.h = PIXEL_SIZE;

    // Render rect
    SDL_RenderFillRect( renderer, &pixel );
    // Render the rect to the screen
    SDL_RenderPresent(renderer);
}

const char* PX = "\u2588\u2588";

// Get chip8 gfx to display buffer
void Display::GetScreen(){
	memcpy(this->screen, chip8->gfx, DISP_X*DISP_Y * sizeof(uint8_t));
}

void Display::DrawScreen(SDL_Renderer* renderer){
	uint16_t x_pos = 1;
	uint16_t y_pos = 1;
	if (chip8->draw_flag){
		chip8->draw_flag = false;
		for (int i = 0; i < DISP_X*DISP_Y; i++){
			if (chip8->gfx[i]){
				DrawPixel(x_pos, y_pos, renderer);
			} else {
				// Instead of clearing a pixel every time, we're using screen as a buffer to check
				// if we actually have to unset it. Drawing unnecessary rectangles is computationally expensive.
				if (this->screen[i]){
					ClearPixel(x_pos, y_pos, renderer);
				}
			}
			if (((i+1) % DISP_X) == 0){
				y_pos++;
				x_pos = 0;
			}
			x_pos++;
		}

		// Display graphics into terminal
		/*
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
		*/
	}
	GetScreen();
}


