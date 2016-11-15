#ifndef GRID_HEADER
#define GRID_HEADER

typedef struct Grid {

	unsigned int width;
	unsigned int height;
	unsigned char **colors;

} Grid;

typedef struct ProcessArgs {

	Grid *grid;
	Grid *tempGrid;
	int windowSize;
	int threadIx;
} ProcessArgs;

Grid *createGrid(unsigned int width, unsigned int height);
void destroyGrid(Grid *grid);
void processGrid(Grid *grid, Grid *tempGrid, int window);
void processGridPthread(void *arg);

#endif