#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "grid.h"
#include "file.h"

#define USE_SDL // use SDL - comment to not use

#ifdef USE_SDL
#include <SDL.h>
#include "render.h"
#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)
#endif

#define ITERATIONS		30000
#define WINDOW			2
#define FILE_NAME		"grid1.txt"

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

#ifdef USE_SDL
	SDL_Event sdlEvent;
	int run = 1;
#endif

	double startTime = omp_get_wtime();

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

	double endTime = omp_get_wtime();
	printf("%f s\n", endTime-startTime);

#ifdef USE_SDL
	destroyRenderer(grid, renderer);
#endif

	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	return 0;
}