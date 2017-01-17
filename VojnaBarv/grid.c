#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "grid.h"
#include <omp.h>
#include "pcg_basic.h"

extern pcg32_random_t rngs[];

// Return pointer to Grid with given dimensions or return NULL.
Grid *createGrid(unsigned int width, unsigned int height) {

	Grid *grid = (Grid *)malloc(sizeof(Grid));
	grid->width = width;
	grid->height = height;
	grid->colors = (unsigned char **)malloc(height * sizeof(unsigned char *));
	if (grid->colors == NULL) {
		printf("createGrid error: could not allocate space.\n");
		return NULL;
	}
	int error = 0;
	for (unsigned int i = 0; i < height; ++i) {
		grid->colors[i] = (unsigned char *)calloc(width, sizeof(unsigned char));
		if (grid->colors[i] == NULL) {
			error = 1;
			break;
		}
	}
	// Dealocate all allocated rows if error.
	if (error) {
		destroyGrid(grid);
		return NULL;
	}
	return grid;
}

// Dealocate given grid.
void destroyGrid(Grid *grid) {

	if (grid == NULL) return;
	for (unsigned int i = 0; i < grid->height; ++i)
		if (grid->colors[i] != NULL)
			free(grid->colors[i]);
	free(grid->colors);
	free(grid);
	grid = NULL;
}

void processGrid(unsigned char* myPart, unsigned char* newRow, unsigned int width, int window, int myId)
	{
	//printf("Sem proces %d\n", myId);

	
	// length of the window to look for neighbors
	int windowSize = (1 + 2 * window);

	// array of cells inside window
	int *neighbors = (int *)calloc( (windowSize * windowSize -1), sizeof(int));

	unsigned int minIdx = window * width;
	unsigned int maxIdx = (window + 1)*width;
	int newIdx = 0;

	for (unsigned int x = minIdx; x < maxIdx; x++, newIdx++) {
		//printf("[%d->", myPart[x]);
		if (myPart[x] == 1) { // if cell is uncolorable(wall)
			newRow[newIdx] = 1;
			//printf("%d] ", newRow[newIdx]);
			continue;
		}
		int index = 0;
		// neighbors left, right
		for (int y = -window; y <= window; y++) {
			if (x + y >= minIdx && x + y < maxIdx) {			// check that window is not out of bounds - x axis
				if (myPart[x+y] != 0							// neighbor must not be blank - 0
					&& (y != 0)									// don't add curent cell to neighbors
					&& myPart[x + y] != 1)						// don't add walls to neighbors
					{
					neighbors[index] = myPart[x + y];
					index++;
				}
			}
		}
		// neighbors up, down
		for (int y = -window; y <= window; y++) {
			for (int j = -window; j <= window; j++) {
				if (y != 0 && (x + y*width + j) >= 0 && (x + y*width + j) < (window * 2 + 1)*width) {			// check that window is not out of bounds - x axis
					//printf("y: %d, j: %d\n", y, j);
					if (myPart[x + y*width + j] != 0							// neighbor must not be blank - 0
						&& myPart[x + y*width + j] != 1)						// don't add walls to neighbors
					{
						// printf("Add %d to neighbors (%d + %d)\n", myPart[x + y*width + j], x, y*width + j);
						neighbors[index] = myPart[x + y*width + j];
						index++;
					}
				}
			}
		}

		if (index > 0) {
			int r = pcg32_boundedrand_r(&rngs, index);
			newRow[newIdx] = neighbors[r];
		}
		else newRow[newIdx] = myPart[x];

		//printf("%d] ", newRow[newIdx]);
	}
	
	free(neighbors);

	
}
