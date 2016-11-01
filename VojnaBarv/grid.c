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
		printf("createGrid error: could not allocate space\n");
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
		freeGrid(grid);
		return NULL;
	}
	return grid;
}

// Dealocate given grid.
void freeGrid(Grid *grid) {

	for (unsigned int i = 0; i < grid->height; ++i)
		free(grid->colors[i]);
	free(grid->colors);
	free(grid);
}