#ifndef GRID_HEADER
#define GRID_HEADER

typedef struct Grid {

	unsigned int width;
	unsigned int height;
	unsigned char **colors;

} Grid;

Grid *createGrid(unsigned int width, unsigned int height);
void destroyGrid(Grid *grid);
unsigned char *transform2DGridTo1D(
	unsigned char **grid,
	unsigned int width,
	unsigned int height);

#endif