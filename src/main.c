#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <SDL.h>

#include "input.h"
#include "sound.h"
#include "screen.h"
#include "ui.h"
#include "pages.h"

const float SCREEN_FPS = 30;
const float SCREEN_TICKS = 1000 / SCREEN_FPS;


int num = 0;

int main(int argc, char* argv[])
{
	// Initialization code
	InitializeScreen();
	InitializeSound();
	InitializePages();

	// Main loop
	while (true) {
		uint64_t start = SDL_GetPerformanceCounter();

		if(HandleInputs()){
			break;
		}

		RenderUI();

		RenderScreen();

		//Uint64 end = SDL_GetPerformanceCounter();

		//float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

		// Cap fps
		//SDL_Delay(floor(SCREEN_TICKS - elapsedMS));

		uint64_t end = SDL_GetPerformanceCounter();

		float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

		printf("elapsed time: %f\n",elapsedMS);

	}
	// Cleanup code
	CleanupScreen();
	//CleanupSound();

	return EXIT_SUCCESS;
}