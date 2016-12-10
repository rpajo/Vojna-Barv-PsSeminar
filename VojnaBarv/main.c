#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "grid.h"
#include "file.h"
#include "pcg_basic.h"

// time measuring is different on different OS
#ifdef _WIN32
#include <Windows.h>
#endif

//#define USE_SDL // use SDL - comment to not use

#ifdef USE_SDL
#include <SDL.h>
#include "render.h"
#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)
#endif

#define ITERATIONS		30000
#define WINDOW			2
#define FILE_NAME		"grid3.txt"

pcg32_random_t rng;

int main(int argc, char **argv) {
	
	GridFile *config = parseFile(FILE_NAME);
	if (config == NULL) return 1;
	Grid *grid = config->initialGrid;
	Grid *tempGrid = createGrid(grid->height, grid->width);
	if (tempGrid == NULL) printf("main error: cannot allocate space for temporary grid.\n");

#ifdef USE_SDL
	Renderer *renderer = initRenderer(WINDOW_NAME, grid, config->cellSize);
	if (renderer == NULL || createTexturesFromColors(config, renderer) != 0 || tempGrid == NULL) {
		destroyRenderer(grid, renderer);
		destroyGrid(grid);
		destroyGrid(tempGrid);
		destroyGridFile(config);
		return 1;
	}
#endif
	
	config->initialGrid = NULL;
	destroyGridFile(config);
	unsigned int iterations = ITERATIONS;
	// Initialize random.
	// Third argument determines the position where sequence of random numbers
	// should start. To give some variation, pointer is assigned which should
	// be nondeterministic.
	pcg32_srandom_r(&rng, (uint64_t)time(NULL), (uint64_t)&rng);

#ifdef _WIN32
	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double elapsedTime;
	
	// get ticks per second
	QueryPerformanceFrequency(&frequency);
	
	/////// TIMER START ///////
	QueryPerformanceCounter(&t1);
#elif defined __gnu_linux__
	struct timespec t1, t2;
	double elapsedTime;
	clock_gettime(CLOCK_REALTIME, &t1);
#endif

#ifdef USE_SDL
	SDL_Event sdlEvent;
	int run = 1;
#endif
	
	while (iterations--) {
		
		processGrid(grid, tempGrid, WINDOW);

#ifdef USE_SDL
		// render grid and display rendered content
		renderGrid(grid, renderer);
		SDL_RenderPresent(renderer->SDLrenderer);

		// handle events
		while (SDL_PollEvent(&sdlEvent))
			if (sdlEvent.type == SDL_QUIT) // window's X button
				run = 0;
		if (!run)
			break;
		// delay in miliseconds
		SDL_Delay(DELAY);
#endif
	}

#ifdef _WIN32
	QueryPerformanceCounter(&t2);
	elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("%f ms\n", elapsedTime);
#elif defined __gnu_linux__
	clock_gettime(CLOCK_REALTIME, &t2);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsedTime += (t2.tv_nsec - t1.tv_nsec) / 1000000.0;
	printf("%f ms \n", elapsedTime);
#endif

#ifdef USE_SDL
	destroyRenderer(grid, renderer);
#endif

	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	return 0;
}
