#ifndef RENDER_HEADER
#define RENDER_HEADER

#include <SDL.h>
#include "grid.h"
#include "file.h"

// Structure that holds all data necessary for rendering grid on window.
typedef struct Renderer {

	SDL_Window *window;
	SDL_Renderer *SDLrenderer;
	SDL_Rect **rects;
	SDL_Texture **textures;
	unsigned int cellSize;

} Renderer;

SDL_Rect **initRects(Grid *grid, unsigned int cellSize);
void destroyRects(Grid *grid, SDL_Rect **rects);
Renderer *initRenderer(const char *windowName, Grid *grid, unsigned int cellSize);
void destroyRenderer(Grid *grid, Renderer *renderer);
SDL_Texture *createTexture(unsigned int size, SDL_Color *color, SDL_Renderer *SDLrenderer);
void renderGrid(Grid *grid, Renderer *renderer);
SDL_Color *createSDLColor(unsigned char r, unsigned char g, unsigned char b);
int createTexturesFromColors(GridFile *config, Renderer *renderer);

#endif