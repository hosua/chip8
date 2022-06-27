#ifndef CLOCK_H
#define CLOCK_H

#include <chip8.h>

#include <chrono>
#include <thread>
#include <functional>
using std::chrono::high_resolution_clock;

extern uint64_t G_ticks_elapsed;

class Clock {
public:
	// Constructor
	Clock() : init_time(high_resolution_clock::now()), tick_start(high_resolution_clock::now()) {}

	void wait(uint16_t num_ticks); // Wait for a certain number of ticks
	void tick(); // Count ticks (should called in the main while loop)
	void print_ticks_elapsed();
private:
	uint32_t seconds_elapsed = 0;
	high_resolution_clock::time_point init_time; 
	high_resolution_clock::time_point tick_start; // The starting time_point of the current tick
};

#endif
