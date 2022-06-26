#include <chrono>
#include <clock.h>
#include <input.h>
#include <iostream>
#include <utility>

// Global tick counter 
uint64_t G_ticks_elapsed = 0;
// TICK is the constant time representing the length of each tick in microseconds. 

// I don't even really know why I implemented my own clock, I just kinda felt like it.
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

// Stop calling thread and wait for a specified number of ticks
void Clock::wait(uint16_t num_ticks){
	high_resolution_clock::time_point wait_start = std::chrono::system_clock::now();
	high_resolution_clock::time_point wait_end = wait_start + std::chrono::microseconds(TICK * num_ticks);
	if (VERBOSE_CLOCK) printf("Clock waiting for %i ticks\n", num_ticks);
	// TODO: This is not right
	// while (wait_start < wait_end);
}


