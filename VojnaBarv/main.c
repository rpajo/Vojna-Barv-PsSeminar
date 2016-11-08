#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "grid.h"
#include "render.h"
#include "file.h"

#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)

int main(int argc, char **argv) {
	
	GridFile *config = parseFile("grid2.txt");
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
	
	SDL_Event sdlEvent;
	int run = 1;
	while (run) {
		
		processGrid(grid, tempGrid);

		// render grid and display rendered content
		renderGrid(grid, renderer);
		SDL_RenderPresent(renderer->SDLrenderer);

		// handle events
		while (SDL_PollEvent(&sdlEvent))
			if (sdlEvent.type == SDL_QUIT) // window's X button
				run = 0;
		// delay in miliseconds
		SDL_Delay(DELAY);
	}
	destroyRenderer(grid, renderer);
	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	return 0;
}