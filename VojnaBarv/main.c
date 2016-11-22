#include <stdio.h>
#include <stdlib.h>
//#include <SDL.h>
//#include "render.h"
#include "grid.h"
#include "file.h"

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <semaphore.h>
#include <Windows.h>

#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)
#define ITERATIONS		30000
#define WINDOW			2

#define NTHREADS		8

unsigned int iterations = ITERATIONS;	
unsigned int nthreads = NTHREADS;
ThreadArgs args;	// main global grid structure
pthread_barrier_t barrier;

int main(int argc, char **argv) {
	
	GridFile *config = parseFile("grid3.txt");
	if (config == NULL) return 1;
	Grid *grid = config->initialGrid;
	Grid *tempGrid = createGrid(grid->height, grid->width);
	if (tempGrid == NULL) printf("main error: cannot allocate space for temporary grid.\n");
	
	printf("GIRD: %d x %d\n", grid->height, grid->width);
	/*Renderer *renderer = initRenderer(WINDOW_NAME, grid, config->cellSize);
	if (renderer == NULL || createTexturesFromColors(config, renderer) != 0 || tempGrid == NULL) {
		destroyRenderer(grid, renderer);
		destroyGrid(grid);
		destroyGrid(tempGrid);
		destroyGridFile(config);
		return 1;
	}*/
	config->initialGrid = NULL;
	destroyGridFile(config);

	// pthreads
	pthread_t threads[NTHREADS];
	if (pthread_barrier_init(&barrier, NULL, NTHREADS))
	{
		printf("Could not create a barrier\n");
		return -1;
	}

	// Timer variables
	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double elapsedTime;

	// get ticks per second
	QueryPerformanceFrequency(&frequency);

	/////// TIMER START ///////
	QueryPerformanceCounter(&t1);

	unsigned int iterations = ITERATIONS;

	
	args.grid = grid;
	args.tempGrid = tempGrid;
	args.window = WINDOW;
	args.barrier = barrier;
	
	//SDL_Event sdlEvent;

	for (int i = 0; i < NTHREADS; i++) {
		pthread_create(&threads[i], NULL, processGridPthread, (void *)i);
	}
		

	for (int i = 0; i<NTHREADS; i++)
		pthread_join(threads[i], NULL);
		
	/*// render grid and display rendered content
	renderGrid(grid, renderer);
	SDL_RenderPresent(renderer->SDLrenderer);

	// handle events
	while (SDL_PollEvent(&sdlEvent))
		if (sdlEvent.type == SDL_QUIT) // window's X button
			run = 0;
	// delay in miliseconds
	SDL_Delay(DELAY);*/


	// end time
	QueryPerformanceCounter(&t2);
	elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("%f ms\n", elapsedTime);

	//destroyRenderer(grid, renderer);
	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	return 0;
}