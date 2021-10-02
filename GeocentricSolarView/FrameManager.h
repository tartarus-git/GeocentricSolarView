#pragma once

#include <thread>
#include <chrono>

class FrameManager {
	std::chrono::high_resolution_clock::time_point frameStart;
	int nsPerFrame;

public:
	FrameManager(int FPS);

	void start();

	std::chrono::high_resolution_clock::duration measure();
	
	float calculateMultiplier();
	void delay();
};
