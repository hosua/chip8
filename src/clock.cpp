#include <clock.h>
#include <input.h>
#include <iostream>

// Global tick counter 
uint64_t G_ticks_elapsed = 0;
// TICK is the constant time representing the length of each tick in microseconds. 

void Clock::tick(){
	if (steady_clock::now() - tick_start > std::chrono::microseconds(TICK)){
		G_ticks_elapsed++;		
		if (VERBOSE_CLOCK) printf("Tick: %zu\n", G_ticks_elapsed);
		if (G_ticks_elapsed % 60 == 0){
			this->seconds_elapsed++;
			if (VERBOSE_CLOCK) printf("Seconds Elapsed: %u\n", this->seconds_elapsed);
		}
		// Start a new tick
		this->tick_start = std::chrono::steady_clock::now();
	}
}

// Stop calling thread and wait for a specified number of ticks
void Clock::wait(uint16_t num_ticks){
	uint16_t last_tick = G_ticks_elapsed + num_ticks + 1;
	if (VERBOSE_CLOCK) printf("Clock waiting for %i ticks\n", num_ticks);
	while (G_ticks_elapsed < last_tick); // Wait
}

