#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "mpi.h"
#include "grid.h"
#include "file.h"
#include "pcg_basic.h"

//#define USE_SDL // use SDL - comment to not use

#ifdef USE_SDL
#include <SDL.h>
#include "render.h"
#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)
#endif

#define ITERATIONS		1
#define WINDOW			1
#define FILE_NAME		"../grid_files/grid1.txt"
#define NTHREADS		2

pcg32_random_t rngs[NTHREADS];
int nthreads = NTHREADS;

int main(int argc, char *argv[]) {
	
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
	omp_set_num_threads(NTHREADS);

	// Initialize random states.
	// Third argument determines the position where sequence of random numbers
	// should start. To give some variation, pointer is assigned which should
	// be nondeterministic.
	for (int i = 0; i < NTHREADS; ++i)
		pcg32_srandom_r(&rngs[i], (uint64_t)time(NULL), (uint64_t)&rngs[i]);

#ifdef USE_SDL
	SDL_Event sdlEvent;
	int run = 1;
#endif

	int myId, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);

	int rows = 2 * WINDOW + 1;
	int partSize = rows * grid->width;

	unsigned char *grid1D = (unsigned char *)calloc(sizeof(unsigned char), grid->width * grid->height + WINDOW * grid->width * 2);
	unsigned char *myPart = (unsigned char *)calloc(sizeof(unsigned char), partSize);

	if (myId == 0) {

		// set border of neutral 1 walls for easier sending of grid chunks
		int end = grid->width * grid->height + WINDOW * grid->width * 2 - 1;
		for (int i = 0; i < WINDOW * grid->width; i++, end--) {
			grid1D[i] = 1;
			grid1D[end] = 1;
		}

		// copy grid to 1d array
		for (unsigned int i = WINDOW; i < grid->height + WINDOW ; i++) {
			for (unsigned int j = 0; j < grid->width; j++) {
				int idx = i*grid->width + j;
				grid1D[idx] = grid->colors[i-WINDOW][j];
			}
		}
	}
	
	while (iterations--) {
		if (myId == 0) {
			int rows = 0;
			while (rows < grid->height) {
				for (int i = 1; i < size && rows < grid->height; i++) {
					printf("Send to %d\n", i);
					MPI_Send(grid1D + rows*grid->width, partSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
					rows++;
				}
			}
			//send end signal
			myPart[0] = 255;
			for (int i = 1; i < size; i++) {
				printf("Send end signal to %d\n", i);
				MPI_Send(myPart, partSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
			}
		}

		else {
			while (1) {
				MPI_Recv(myPart, partSize, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				if (myPart[0] == 255) {
					break;
				}
				else {
					printf("Recieved id: %d\n", myId);
					// call process grid

					
					for (int i = 0; i < partSize; i++) {
						printf("%d ", myPart[i]);
					}
					
				}
			}
		}



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


	//double startTime = omp_get_wtime();



	MPI_Finalize();
	//double endTime = omp_get_wtime();
	//printf("%f s\n", endTime-startTime);

#ifdef USE_SDL
	destroyRenderer(grid, renderer);
#endif

	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	free(grid1D);
	free(myPart);

	return 0;
}
