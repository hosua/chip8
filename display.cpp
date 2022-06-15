#include <chip8.h>
#include <display.h>
#include <cpu.h>

const char* PX = "\u2588\u2588"; 


void Display::DrawScreen(){
	if (chip8->draw_flag) {
		chip8->draw_flag = false;

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

	
}


