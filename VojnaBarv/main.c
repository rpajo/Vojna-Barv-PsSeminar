#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "grid.h"
#include "render.h"
#include <Windows.h>

#define WINDOW_NAME		"Vojna Barv"
#define GRID_WIDTH		((unsigned int) 200)
#define GRID_HEIGHT		((unsigned int) 200)
#define CELL_SIZE		((unsigned int) 8)
#define NUM_COLORS		5
#define DELAY			((unsigned int) 50)
#define WINDOW			((unsigned int) 2)
#define ITERATIONS		((unsigned int) 500);

int main(int argc, char **argv) {

	////////////////////////////// Timer initialization  /////////////////////////////////////

	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double elapsedTime;

	// get ticks per second
	QueryPerformanceFrequency(&frequency);


	//////////////////// TUKAJ KREIRAS IN NASTAVIS ZACETNO STANJE BARV V MREZI //////////////
	Grid *grid = createGrid(GRID_WIDTH, GRID_HEIGHT); // po defaultu so vse celice barve 0

	//////////////////////////////////////////////////////////////////////////////////////////

	Renderer *renderer = initRenderer(WINDOW_NAME, grid, CELL_SIZE);
	if (renderer == NULL) {
		freeGrid(grid);
		return 1;
	}

	// table of textures
	renderer->textures = (SDL_Texture **)malloc(NUM_COLORS * sizeof(SDL_Texture *));
	// Add textures for white and black squares.
	SDL_Color *white = createSDLColor(255, 255, 255);	// uncolored cell
	SDL_Color *black = createSDLColor(0, 0, 0);			// wall cell
	SDL_Color *green = createSDLColor(0, 255, 100);		// normal color cell - greed
	SDL_Color *blue = createSDLColor(0, 191, 255);		// normal color cell - blue
	renderer->textures[0] = createTexture(renderer->cellSize, white, renderer->SDLrenderer);
	renderer->textures[1] = createTexture(renderer->cellSize, green, renderer->SDLrenderer);
	renderer->textures[2] = createTexture(renderer->cellSize, blue, renderer->SDLrenderer);
	renderer->textures[3] = createTexture(renderer->cellSize, black, renderer->SDLrenderer);


	free(white);
	free(black);
	free(green);
	free(blue);
	// color pixels for demo
	grid->colors[0][0] = 1;
	grid->colors[GRID_HEIGHT-1][GRID_WIDTH-1] = 2;


	// allocate space for new color gird for transitioning between iterations
	unsigned char **newGrid = createNewGrid(grid->height, grid->width);

	SDL_Event sdlEvent;
	int run = 1;

	/////// TIMER START ///////
	QueryPerformanceCounter(&t1);

	unsigned int iterations = ITERATIONS;
	while (run && iterations) {

		// This line is currently not necessary as we redraw all pixels every cycle
		SDL_RenderClear(renderer->SDLrenderer);

		//////////////////// SEM VSTAVIS KODO ZA MANIPULIRANJE Grid STRUKTURE ////////////
		
		// process color grid and update it
		processGrid(grid, newGrid, WINDOW); // arguments: pointer to grid structure, pointer to temporary color grid, window size
		
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
		
		iterations--;
	}

	// end time
	QueryPerformanceCounter(&t2);
	elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("%f ms\n", elapsedTime);

	destroyRenderer(grid, renderer);
	return 0;
}