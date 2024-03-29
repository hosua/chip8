#include <chrono>
#include <clock.h>
#include <input.h>
#include <iostream>
#include <utility>

// Global tick counter 
uint64_t G_ticks_elapsed = 0;

// TICK is the constant time representing the length of each tick in microseconds. 
void Clock::tick(){
	if (high_resolution_clock::now() - tick_start > std::chrono::microseconds(TICK)){
		G_ticks_elapsed++;		
		if (VERBOSE_CLOCK) printf("Tick: %zu\n", G_ticks_elapsed);
		if (G_ticks_elapsed % 60 == 0){
			this->seconds_elapsed++;
			if (VERBOSE_CLOCK) printf("Seconds Elapsed: %u\n", this->seconds_elapsed);
		}
		// Start a new tick
		this->tick_start = std::chrono::high_resolution_clock::now();
	}
}

// Stop calling thread for specified number of ticks
void Clock::wait(uint16_t num_ticks){
	if (VERBOSE_CLOCK) printf("Clock waiting for %i ticks\n", num_ticks);
	std::this_thread::sleep_for(std::chrono::microseconds(TICK * num_ticks));

}


