#include <stdio.h>
#include <stdlib.h>
#include "grid.h"

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

// Return 1D copy of 2D array or NULL on error
unsigned char *transform2DGridTo1D(unsigned char **grid,
	unsigned int width,
	unsigned int height) {
	
	// allocate new array
	unsigned char *newGrid = (unsigned char *)malloc(width * height * sizeof(unsigned char));
	if (newGrid == NULL)
		return NULL;

	// copy array
	int ix = 0; // for indexing 1D array
	for (unsigned int i = 0; i < height; ++i)
		for (unsigned int j = 0; j < width; ++j)
			newGrid[ix++] = grid[i][j];
	return newGrid;
}