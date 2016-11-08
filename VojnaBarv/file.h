#ifndef FILE_HEADER
#define FILE_HEADER

#include "grid.h"
#include <SDL.h>

typedef struct GridFile {

	int width;
	int height;
	int cellSize;
	Grid *initialGrid;
	int numColors;
	SDL_Color **colors;

} GridFile;

GridFile *parseFile(char *filename);
int parseFileType(FILE *file);
int parseDimensions(FILE *file, GridFile *config);
int parseColors(FILE *file, GridFile *config);
int parseFileType0(FILE *file, GridFile *config);
int parseFileType1(FILE *file, GridFile *config);
void destroyGridFile(GridFile *config);

#endif