#include "mwc64x.cl"

__kernel void processGrid(
	__global uchar *grid,
	__global uchar *tempGrid,
	int width,
	int height,
	int numColors,
	int seed,
	int window) {
	
	// global index of cell
	uint ix = get_global_id(0);

	// state for rng
	mwc64x_state_t rst;

	// create counters for each (non-blank/non-wall) color
	// It's not allowed to make array with non-static size so we have array of
	// 32 elements. The host program is responsible for limiting colors to 32
	// and taking care that grid has right range of colors.
	int colorCounters[32];
	// counter for all non-blank/wall squares (needed for determining random range)
	int countableSquares;
	
	while (ix < (width * height))
	{
		// initializing all counters
		countableSquares = 0; // needed for determining random range
		for (int i = 0; i < numColors; ++i) {
			colorCounters[i] = 0;
		}
		
		// optimization if computing division and remainder once?
		int ixDivWidth = (ix / width);
		int ixRemWidth = (ix % width);

		// optimization - calculating loop indexes (stop indexes) once
		int startI = ixDivWidth - window;
		int endI = ixDivWidth + window;
		int startJ = ixRemWidth - window;
		int endJ = ixRemWidth + window;

		for (int i = startI; i <= endI; ++i) {
			for (int j = startJ; j <= endJ; ++j) {
				if (i >= 0 && j >= 0 && i < height && j < width) {
					int color = grid[i * width + j];
					if (color > 1) {
						colorCounters[color]++;
						countableSquares++;
					}
				}
			}
		}
		if (grid[ix] == 1 || countableSquares == 0) { // ignore wall
			tempGrid[ix] = grid[ix];
			ix += get_global_size(0);
			continue;
		}

		// init rng state and get random number
		rst.x = seed + ix;
		rst.c = seed;
		MWC64X_NextUint(&rst);

		// determine the color of square
		int randNum = (rst.x % countableSquares) + 1;
		int i = 1;
		while (randNum > 0) {
			i++;
			randNum -= colorCounters[i];
		}
		tempGrid[ix] = i;
		ix += get_global_size(0);
	}
}