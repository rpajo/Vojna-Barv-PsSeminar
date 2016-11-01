#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "grid.h"
#include "render.h"

#define WINDOW_NAME		"Vojna Barv"
#define GRID_WIDTH		((unsigned int) 16)
#define GRID_HEIGHT		((unsigned int) 9)
#define CELL_SIZE		((unsigned int) 20)
#define NUM_COLORS		2
#define DELAY			((unsigned int) 50)

int main(int argc, char **argv) {
	
	//////////////////// TUKAJ KREIRAS IN NASTAVIS ZACETNO STANJE BARV V MREZI //////////////
	Grid *grid = createGrid(GRID_WIDTH, GRID_HEIGHT); // po defaultu so vse celice barve 0
	int pointX = 15; // spremenljivki za demo
	int pointY = 8;
	//////////////////////////////////////////////////////////////////////////////////////////

	Renderer *renderer = initRenderer(WINDOW_NAME, grid, CELL_SIZE);
	if (renderer == NULL) {
		freeGrid(grid);
		return 1;
	}

	// table of textures
	renderer->textures = (SDL_Texture **)malloc(NUM_COLORS * sizeof(SDL_Texture *));
	// Add textures for white and black squares.
	SDL_Color *white = createSDLColor(255, 255, 255);
	SDL_Color *black = createSDLColor(0, 0, 0);
	renderer->textures[0] = createTexture(renderer->cellSize, white, renderer->SDLrenderer);
	renderer->textures[1] = createTexture(renderer->cellSize, black, renderer->SDLrenderer);
	free(white);
	free(black);

	SDL_Event sdlEvent;
	int run = 1;
	while (run) {

		// This line is currently not necessary as we redraw all pixels every cycle
		SDL_RenderClear(renderer->SDLrenderer);

		//////////////////// SEM VSTAVIS KODO ZA MANIPULIRANJE Grid STRUKTURE ////////////
		grid->colors[pointY][pointX] = 0;
		pointX = (pointX + 1) % GRID_WIDTH;
		if (!pointX)
			pointY = (pointY + 1) % GRID_HEIGHT;
		grid->colors[pointY][pointX] = 1;
		/////////////////////////////////////////////////////////////////////////////////

		// render grid and display rendered content
		renderGrid(grid, renderer);
		SDL_RenderPresent(renderer->SDLrenderer);

		// handle events
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT) // window's X button
				run = 0;
		}
		// delay in miliseconds
		SDL_Delay(DELAY);
	}
	destroyRenderer(grid, renderer);
	return 0;
}