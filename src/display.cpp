#include <chip8.h>
#include <display.h>
#include <cpu.h>

// Draw pixel into renderer
void Display::DrawPixel(uint16_t x, uint16_t y, SDL_Renderer *renderer){
	// Set render color to white ( rect will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
	// Create a 10x10 rectange (the pixel)
    SDL_Rect r;
    r.x = x * PIXEL_SIZE;
    r.y = y * PIXEL_SIZE;

    r.w = PIXEL_SIZE;
    r.h = PIXEL_SIZE;

    // Render rect
    SDL_RenderFillRect( renderer, &r );
    // Render the rect to the screen
    SDL_RenderPresent(renderer);
}

void Display::ClearPixel(uint16_t x, uint16_t y, SDL_Renderer *renderer){
	// Set render color to white ( rect will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
	// Create a 10x10 rectange (the pixel)
    SDL_Rect r;
    r.x = x * PIXEL_SIZE;
    r.y = y * PIXEL_SIZE;

    r.w = PIXEL_SIZE;
    r.h = PIXEL_SIZE;

    // Render rect
    SDL_RenderFillRect( renderer, &r );
    // Render the rect to the screen
    SDL_RenderPresent(renderer);
}

void Display::ClearScreen(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = DISP_X * PIXEL_SIZE + PIXEL_SIZE;
	r.h = DISP_Y * PIXEL_SIZE + PIXEL_SIZE;
	SDL_RenderFillRect(renderer, &r);
	SDL_RenderPresent(renderer);
}

const char* PX = "\u2588\u2588";

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
				// if we actually have to unset it. Drawing rectangles is computationally expensive.
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
	GetScreen();
}


