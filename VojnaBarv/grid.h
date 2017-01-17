#ifndef GRID_HEADER
#define GRID_HEADER

typedef struct Grid {

	unsigned int width;
	unsigned int height;
	unsigned char **colors;

} Grid;

Grid *createGrid(unsigned int width, unsigned int height);
void destroyGrid(Grid *grid);
void processGrid(unsigned char* myPart, unsigned char* newRow, unsigned int width, int window, int myId);

#endif