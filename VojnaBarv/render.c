#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "grid.h"
#include "render.h"
#include "file.h"

// Return pointer to matrix of initialized rects or NULL on error.
SDL_Rect **initRects(Grid *grid, unsigned int cellSize) {

	SDL_Rect **rects = (SDL_Rect **)malloc(grid->height * sizeof(SDL_Rect *));
	if (rects == NULL) {
		printf("initRects error: could not allocate space\n");
		return NULL;
	}
	int error = 0;
	for (unsigned int i = 0; i < grid->height; ++i) {
		rects[i] = (SDL_Rect *)malloc(grid->width * sizeof(SDL_Rect));
		if (rects[i] == NULL) {
			error = 1;
			break;
		}
		for (unsigned int j = 0; j < grid->width; ++j) {
			// set position of topleft corner, width and height of rectangle
			rects[i][j].x = j * cellSize;
			rects[i][j].y = i * cellSize;
			rects[i][j].w = cellSize;
			rects[i][j].h = cellSize;
		}
	}
	// Dealocate all allocated rows if error.
	if (error) {
		destroyRects(grid, rects);
		return NULL;
	}
	return rects;
}

// Free memory alocated for rects.
void destroyRects(Grid *grid, SDL_Rect **rects) {

	for (unsigned int i = 0; i < grid->height; ++i)
		free(rects[i]);
	free(rects);
	rects = NULL;
}

// Return pointer to texture (square) of given size and color or NULL on error.
SDL_Texture *createTexture(unsigned int size, RGBColor *color, SDL_Renderer *SDLrenderer) {

	// Create surface, use 32-bit depth with default masks.
	SDL_Surface *surface = SDL_CreateRGBSurface(0, size, size, 32, 0, 0, 0, 0);
	if (surface == NULL) {
		printf("SDL_CreateRGBSurface error: %s\n", SDL_GetError());
		return NULL;
	}
	// Paint surface with given color.
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, color->r, color->g, color->b));
	// Create texture from surface.
	SDL_Texture *texture = SDL_CreateTextureFromSurface(SDLrenderer, surface);
	if (texture == NULL) {
		printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
	}
	SDL_FreeSurface(surface);
	return texture;
}

// Render grid.
void renderGrid(Grid *grid, Renderer *renderer) {
	
	for (unsigned int i = 0; i < grid->height; ++i)
		for (unsigned int j = 0; j < grid->width; ++j)
			SDL_RenderCopy(renderer->SDLrenderer, renderer->textures[grid->colors[i][j]], NULL, &(renderer->rects[i][j]));
}

// Return pointer to initialized Renderer or NULL on error.
Renderer *initRenderer(const char *windowName, Grid *grid, unsigned int cellSize) {
	
	Renderer *renderer = (Renderer *)malloc(sizeof(Renderer));
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return NULL;
	}
	renderer->window = SDL_CreateWindow(windowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, grid->width * cellSize, grid->height * cellSize, 0);
	if (renderer->window == NULL) {
		printf("SDL_CreateWindow error: %s\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}
	renderer->SDLrenderer = SDL_CreateRenderer(renderer->window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer->SDLrenderer == NULL) {
		printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
		SDL_DestroyWindow(renderer->window);
		SDL_Quit();
		return NULL;
	}
	renderer->rects = initRects(grid, cellSize);
	if (renderer->rects == NULL) {
		printf("initRects error: could not initialize rects\n");
		SDL_DestroyRenderer(renderer->SDLrenderer);
		SDL_DestroyWindow(renderer->window);
		SDL_Quit();
		return NULL;
	}
	renderer->cellSize = cellSize;
	return renderer;
}

// Release all SDL resources.
void destroyRenderer(Grid *grid, Renderer *renderer) {
	if (renderer == NULL) return;
	destroyRects(grid, renderer->rects);
	SDL_DestroyRenderer(renderer->SDLrenderer); // textures are also destroyed in this call
	SDL_DestroyWindow(renderer->window);
	SDL_Quit();
	free(renderer);
	renderer = NULL;
}

// Return pointer to SDL_Color or NULL on error.
SDL_Color *createSDLColor(unsigned char r, unsigned char g, unsigned char b) {

	SDL_Color *color = (SDL_Color *)malloc(sizeof(SDL_Color));
	if (color == NULL) return NULL;
	color->r = r;
	color->g = g;
	color->b = b;
	color->a = 0;
	return color;
}

// Return pointer to array of pointers to textures or NULL on error.
int createTexturesFromColors(GridFile *config, Renderer *renderer) {
	
	renderer->textures = (SDL_Texture **)malloc(config->numColors * sizeof(SDL_Texture *));
	if (renderer->textures == NULL) {
		printf("createTexturesFromColors error: cannot allocate space.\n");
		return 1;
	}
	for (int i = 0; i < config->numColors; ++i) {
		renderer->textures[i] = createTexture(renderer->cellSize, &config->colors[i], renderer->SDLrenderer);
		if (renderer->textures[i] == NULL) {
			return 1;
		}
	}
	return 0;
}