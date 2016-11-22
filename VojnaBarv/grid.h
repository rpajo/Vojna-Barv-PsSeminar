#ifndef GRID_HEADER
#define GRID_HEADER

#define NTHREADS	4

typedef struct Grid {

	unsigned int width;
	unsigned int height;
	unsigned char **colors;

} Grid;

typedef struct ThreadArgs {

	Grid *grid;
	Grid *tempGrid;
	unsigned int window;
	unsigned int ix;
	pthread_barrier_t *barrier;

} ThreadArgs;

Grid *createGrid(unsigned int width, unsigned int height);
void destroyGrid(Grid *grid);
void processGrid(Grid *grid, Grid *tempGrid, int window);
void processGridPthread(void *arg);

#endif