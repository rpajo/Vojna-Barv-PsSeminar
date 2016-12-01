#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "grid.h"

#define COLOR_FORMAT		"%d,%d,%d"
#define COORDINATE_FORMAT	"%d,%d"

GridFile *parseFile(char *filename) {

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("parseFile error: error opening file.\n");
		return NULL;
	}
	int fileType, error = 0;
	if ((fileType = parseFileType(file)) == -1) return NULL;
	
	GridFile *config = (GridFile *)malloc(sizeof(GridFile));
	if (config == NULL) {
		printf("parseFile error: cannot allocate GridFile.\n");
		return NULL;
	}
	error = parseDimensions(file, config);
	if (!error) {
		error = parseColors(file, config);
	}
	config->initialGrid = createGrid(config->width, config->height);
	if (config->initialGrid == NULL) {
		error = 1;
	}
	if (!error) {
		if (fileType == 0)
			error = parseFileType0(file, config);
		else
			error = parseFileType1(file, config);
	}
	if (error) {
		destroyGridFile(config);
		return NULL;
	}
	/* DEBUGGING
	printf("Read config:\n");
	printf("File Type: %d\n", fileType);
	printf("width=%d, height=%d, cellSize=%d\n", config->width, config->height, config->cellSize);
	printf("%d colors (r, g, b): ", config->numColors);
	for (int i = 0; i < config->numColors; ++i)
		printf("(%d, %d, %d) ", config->colors[i]->r, config->colors[i]->g, config->colors[i]->b);
	printf("\n");
	printf("Grid:\n");
	for (int i = 0; i < config->height; ++i) {
		for (int j = 0; j < config->width; ++j) {
			printf("%d ", config->initialGrid->colors[i][j]);
		}
		printf("\n");
	}
	*/

	fclose(file);
	return config;
}

// Retrun file type or -1 on error.
int parseFileType(FILE *file) {

	int fileType;
	if (fscanf(file, "%d", &fileType) == 0) {
		printf("parseFileType error: cannot read file type.\n");
		return -1;
	}
	if (fileType < 0 || fileType > 1) {
		printf("parseFileType error: file type %d not supported.\n", fileType);
		return -1;
	}
	return fileType;
}

// Write dimensions from file to structure or return nonzero on error.
int parseDimensions(FILE *file, GridFile *config) {

	if (fscanf(file, "%d", &config->width) == 0) {
		printf("parseDimensions error: could not read width.\n");
		return 1;
	}
	if (fscanf(file, "%d", &config->height) == 0) {
		printf("parseDimensions error: could not read height.\n");
		return 1;
	}
	if (fscanf(file, "%d", &config->cellSize) == 0) {
		printf("parseDimensions error: could not read cell size.\n");
		return 1;
	}
	return 0;
}

// Allocate array of RGBColors or set NULL pointer to colors in config on error.
//
// Function tries to allocate space for all colors. If error happens any time
// in process then all allocated space is freed and pointer set to NULL.
// Caller should check if colors pointer is NULL.
int parseColors(FILE *file, GridFile *config) {

	if (fscanf(file, "%d", &config->numColors) == 0) {
		printf("parseColors error: could not read number of colors.\n");
		return 1;
	}
	if (config->numColors <= 0) {
		printf("parseColors error: number of colors less than 1.\n");
		return 1;
	}
	config->colors = (RGBColor *)malloc(config->numColors * sizeof(RGBColor));
	if (config->colors == NULL) {
		printf("parseColors error: could not allocate array of colors.\n");
		return 1;
	}
	unsigned int r, g, b;
	for (int i = 0; i < config->numColors; ++i) {
		if (fscanf(file, COLOR_FORMAT, &r, &g, &b) <= 0) {
			printf("parseColors error: could not read color %d.\n", i);
			free(config->colors);
			config->colors = NULL;
			return 1;
		}
		else {
			config->colors[i].r = r;
			config->colors[i].g = g;
			config->colors[i].b = b;
		}
	}
	return 0;
}

// Read width * height numbers to grid and return 0. If error, return 1 immediately.
int parseFileType0(FILE * file, GridFile * config) {
	
	int color;
	for (int i = 0; i < config->height; ++i) {
		for (int j = 0; j < config->width; ++j) {
			if (fscanf(file, "%d", &color) == 0) {
				printf("parseFileType0 error: cannot read into colors[%d][%d].\n", i, j);
				return 1;
			}
			if (color < 0 || color >= config->numColors) {
				printf("parseFileType0 error: colors[%d][%d] is not in range.\n", i, j);
				return 1;
			}
			config->initialGrid->colors[i][j] = color;
		}
	}
	return 0;
}

// Read coordinates of squares of each color. Return nonzero on error.
//
// If the same coordinate is used for several colors, the last color is taken.
int parseFileType1(FILE * file, GridFile * config) {
	
	int numCoordinates;
	for (int i = 0; i < config->numColors; ++i) {
		if (fscanf(file, "%d", &numCoordinates) == 0) {
			printf("parseFileType1 error: cannot read number of coordinates for color %d.\n", i);
			return 1;
		}
		if (numCoordinates < 0) {
			printf("parseFileType1 error: number of coordinates must be nonnegative.\n");
			return 1;
		}
		else if (numCoordinates == 0) {
			continue;
		}
		int x, y;
		for (int j = 0; j < numCoordinates; ++j) {
			if (fscanf(file, COORDINATE_FORMAT, &x, &y) == 0) {
				printf("parseFileType1 error: could not read coordinates.\n");
				return 1;
			}
			if (x < 0 || x >= config->width || y < 0 || y > config->height) {
				printf("parseFileType1 error: coordinates not in range.\n");
				return 1;
			}
			config->initialGrid->colors[y][x] = i;
		}
	}
	return 0;
}

// Release all resources.
void destroyGridFile(GridFile *config) {

	if (config->colors != NULL)
		free(config->colors);
	if (config->initialGrid != NULL)
		destroyGrid(config->initialGrid);
	free(config);
	config = NULL;
}
