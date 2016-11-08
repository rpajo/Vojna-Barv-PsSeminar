#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "grid.h"
#include "render.h"
#include "file.h"
#include <Windows.h>

#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)
#define ITERATIONS		500
#define WINDOW			2

int main(int argc, char **argv) {
	
	GridFile *config = parseFile("grid1.txt");
	if (config == NULL) return 1;
	Grid *grid = config->initialGrid;
	Grid *tempGrid = createGrid(grid->height, grid->width);
	if (tempGrid == NULL) printf("main error: cannot allocate space for temporary grid.\n");
	Renderer *renderer = initRenderer(WINDOW_NAME, grid, config->cellSize);
	if (renderer == NULL || createTexturesFromColors(config, renderer) != 0 || tempGrid == NULL) {
		destroyRenderer(grid, renderer);
		destroyGrid(grid);
		destroyGrid(tempGrid);
		destroyGridFile(config);
		return 1;
	}
	config->initialGrid = NULL;
	destroyGridFile(config);

	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double elapsedTime;

	// get ticks per second
	QueryPerformanceFrequency(&frequency);

	/////// TIMER START ///////
	QueryPerformanceCounter(&t1);

	unsigned int iterations = ITERATIONS;
	
	SDL_Event sdlEvent;
	int run = 1;
	while (run && iterations) {
		
		processGrid(grid, tempGrid, WINDOW);
		
		// render grid and display rendered content
		renderGrid(grid, renderer);
		SDL_RenderPresent(renderer->SDLrenderer);

		// handle events
		while (SDL_PollEvent(&sdlEvent))
			if (sdlEvent.type == SDL_QUIT) // window's X button
				run = 0;
		// delay in miliseconds
		SDL_Delay(DELAY);
		
		iterations--;
	}

	// end time
	QueryPerformanceCounter(&t2);
	elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("%f ms\n", elapsedTime);

	destroyRenderer(grid, renderer);
	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	return 0;
}