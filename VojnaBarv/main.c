#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

#define ITERATIONS		30000
#define WINDOW			1
#define FILE_NAME		"../grid_files/grid3.txt"

pcg32_random_t rngs;


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

#ifdef USE_SDL
	SDL_Event sdlEvent;
	int run = 1;
#endif
	int myId, size;
	double start, finish;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);

	// Initialize random states.
	// Third argument determines the position where sequence of random numbers
	// should start. To give some variation, pointer is assigned which should
	// be nondeterministic.
	
	pcg32_srandom_r(&rngs, (uint64_t)time(NULL), (uint64_t)&rngs);

	int rows = 2 * WINDOW + 1;
	int partSize = rows * grid->width;

	unsigned char *grid1D = (unsigned char *)calloc(sizeof(unsigned char), grid->width * grid->height + WINDOW * grid->width * 2);
	unsigned char *grid1Dtemp = (unsigned char *)calloc(sizeof(unsigned char), grid->width * grid->height + WINDOW * grid->width * 2);
	unsigned char *myPart = (unsigned char *)calloc(sizeof(unsigned char), partSize);
	unsigned char *newRow = (unsigned char *)calloc(sizeof(unsigned char), grid->width);
	unsigned char* tempPointer;		// temp pointer to switch grids

	// set border of neutral 1 walls for easier sending of grid chunks
	int end = grid->width * grid->height + WINDOW * grid->width * 2 - 1;
	for (unsigned int i = 0; i < WINDOW * grid->width; i++, end--) {
		grid1D[i] = 1;
		grid1D[end] = 1;
		grid1Dtemp[i] = 1;
		grid1Dtemp[end] = 1;
	}

	// copy grid to 1d array
	for (unsigned int i = WINDOW; i < grid->height + WINDOW; i++) {
		for (unsigned int j = 0; j < grid->width; j++) {
			int idx = i*grid->width + j;
			grid1D[idx] = grid->colors[i - WINDOW][j];
		}
	}


	start = MPI_Wtime(); /*start timer*/

	while (iterations--) {
		if (myId == 0) {

			int updateRow = 0;
			unsigned int sendRows = 0, recvRows = 0;

			while (sendRows < grid->height) {
				for (int i = 1; i < size && sendRows < grid->height; i++) {
					//printf("Send to %d\n", i);
					MPI_Send(grid1D + sendRows*grid->width, partSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
					sendRows++;
					recvRows++;
				}

				for (unsigned int i = 1; i <= recvRows; i++) {
					//printf("%d\n", (updateRow + 1)*grid->width);
					MPI_Recv(grid1Dtemp +(updateRow+1)*grid->width, grid->width, MPI_UNSIGNED_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					updateRow++;
					/*
					printf("\n");
					for (unsigned int j = 0; j < grid->width; j++) {
						printf("%d ", grid->colors[updateRow - 1][j]);
					}
					*/
				}
				recvRows = 0;
			}
			
			//send end signal
			myPart[0] = 255;
			for (int i = 1; i < size; i++) {
				//printf("Send end signal to %d\n", i);
				MPI_Send(myPart, partSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
			}

			tempPointer = grid1D;
			grid1D = grid1Dtemp;
			grid1Dtemp = tempPointer;

			/*
			for (int y = 0; y < grid->height + 2 * WINDOW; y++) {
				for (int x = 0; x < grid->width; x++) {
					printf("%d ", grid1D[y*grid->width + x]);
				}
				printf("\n");
			}
			*/


		}

		else {
			while (1) {
				MPI_Recv(myPart, partSize, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				if (myPart[0] == 255) {
					break;
				}
				else {
					/*printf("Recieved id: %d\n", myId);
					for (int i = 0; i < partSize; i++) {
						if (i == WINDOW*grid->width || i == (WINDOW + 1)*grid->width + 1) printf("| ");
						printf("%d ", myPart[i]);
					}*/
					//printf("\n");
					// call process grid
					processGrid(myPart, newRow, grid->width, WINDOW, myId);
					//printf("\Sending Data back to master\n");
					MPI_Send(newRow, grid->width, MPI_UNSIGNED_CHAR, 0, myId, MPI_COMM_WORLD);
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

	finish = MPI_Wtime(); /*stop timer*/

	if (myId == 0) printf("%f s\n", finish - start);
	
	MPI_Finalize();


#ifdef USE_SDL
	destroyRenderer(grid, renderer);
#endif

	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	free(grid1D);
	free(myPart);

	return 0;
}
