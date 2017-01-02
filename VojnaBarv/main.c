#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // uint types
#include <time.h>
#include "grid.h"
#include "file.h"

// OpenCL
#define _CRT_SECURE_NO_WARNINGS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "CL\cl.h"

// Windows time measurement
#ifdef _WIN32
#include <Windows.h>
#endif

//#define USE_SDL // use SDL - comment to not use
#ifdef USE_SDL
#include <SDL.h>
#include "render.h"
#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)
#endif

// parameters
#define ITERATIONS		30000
#define WINDOW			2
#define FILE_NAME		"grid3.txt"
#define WORKGROUP_SIZE	(512)
#define MAX_SOURCE_SIZE	16384
#define PLATFORM_ID		0
#define DEVICE_ID		0

// for debugging
void printGrid(unsigned char *grid1D, int width, int height) {
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			printf("%d ", (int)grid1D[i * width + j]);
		}
		printf("\n");
	}
}

int main(int argc, char **argv) {
	
	GridFile *config = parseFile(FILE_NAME);
	if (config == NULL) return 1;
	Grid *grid = config->initialGrid; // transfer grid ownership
	config->initialGrid = NULL;

#ifdef USE_SDL
	Renderer *renderer = initRenderer(WINDOW_NAME, grid, config->cellSize);
	if (renderer == NULL || createTexturesFromColors(config, renderer) != 0) {
		destroyRenderer(grid, renderer);
		destroyGrid(grid);
		destroyGridFile(config);
		return 1;
	}
#endif

	unsigned int iterations = ITERATIONS;
	int gridSize = grid->width * grid->height;
	unsigned char *oneDGrid = transform2DGridTo1D(grid->colors, grid->width, grid->height);

#ifdef USE_SDL
	SDL_Event sdlEvent;
	int run = 1;
#endif

	// read kernel source
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("processGrid.cl", "r");
	if (!fp)
	{
		fprintf(stderr, ":-(#\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);

	// Kernel arguments - window and a seed for every iteration - seeds are
	// generated randomly. Seed for rng is current time and uninitialized
	// value from memory.
	double *randFromMem = (double *)malloc(sizeof(double));
	pcg32_srandom((uint64_t) time(NULL), (uint64_t) *randFromMem);
	free(randFromMem);
	int *seeds = (int *) malloc(ITERATIONS * sizeof(int));
	for (int i = 0; i < ITERATIONS; ++i)
		seeds[i] = pcg32_random();
	int window = WINDOW;

	cl_int ret; // for return values
	// get OpenCL platforms
	cl_platform_id	platform_id[10];
	cl_uint			ret_num_platforms;
	ret = clGetPlatformIDs(10, platform_id, &ret_num_platforms);
	// get devices for platform
	cl_device_id	device_id[10];
	cl_uint			ret_num_devices;
	ret = clGetDeviceIDs(platform_id[PLATFORM_ID], CL_DEVICE_TYPE_ALL, 10,
		device_id, &ret_num_devices);
	// create context, command queue
	cl_context context = clCreateContext(NULL, 1, &device_id[0], NULL, NULL, &ret);
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);
	// allocate space for grid and temp grid on device
	// we copy the same grid twice so that it doesn't matter which kernel (1 or 2) is
	// run first
	cl_mem grid1 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
		gridSize * sizeof(unsigned char), oneDGrid, &ret);
	cl_mem grid2 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
		gridSize * sizeof(unsigned char), oneDGrid, &ret);

	// build program and show log
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
		NULL, &ret);
	ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);
	size_t build_log_len;
	char *build_log;
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG,
		0, NULL, &build_log_len);
	build_log = (char *)malloc(sizeof(char)*(build_log_len + 1));
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG,
		build_log_len, build_log, NULL);
	printf("%s\n", build_log);
	free(build_log);
	free(source_str);

	// create 2 kernels, get kernel workgroup info, set kernel args
	// http://stackoverflow.com/questions/11041081/how-to-effectively-swap-opencl-memory-buffers
	// suggests to create 2 kernels with swapped arguments rather than setting swapped arguments
	// every single iteration
	cl_kernel kernel1 = clCreateKernel(program, "processGrid", &ret);
	cl_kernel kernel2 = clCreateKernel(program, "processGrid", &ret);
	// get kernel workgroup info
	size_t buf_size_t;
	clGetKernelWorkGroupInfo(kernel1, device_id[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buf_size_t), &buf_size_t, NULL);
	printf("thread num multiple = %d", buf_size_t);
	// wait for enter
	char ch; 
	scanf("%c", &ch);

	// kernel 1
	ret = clSetKernelArg(kernel1, 0, sizeof(cl_mem), (void *)&grid1);
	ret |= clSetKernelArg(kernel1, 1, sizeof(cl_mem), (void *)&grid2);
	ret |= clSetKernelArg(kernel1, 2, sizeof(cl_int), (void *)&grid->width);
	ret |= clSetKernelArg(kernel1, 3, sizeof(cl_int), (void *)&grid->height);
	ret |= clSetKernelArg(kernel1, 4, sizeof(cl_int), (void *)&config->numColors);
	ret |= clSetKernelArg(kernel1, 6, sizeof(cl_int), (void *)&window);
	// kernel 2
	ret = clSetKernelArg(kernel2, 0, sizeof(cl_mem), (void *)&grid2);
	ret |= clSetKernelArg(kernel2, 1, sizeof(cl_mem), (void *)&grid1);
	ret |= clSetKernelArg(kernel2, 2, sizeof(cl_int), (void *)&grid->width);
	ret |= clSetKernelArg(kernel2, 3, sizeof(cl_int), (void *)&grid->height);
	ret |= clSetKernelArg(kernel2, 4, sizeof(cl_int), (void *)&config->numColors);
	ret |= clSetKernelArg(kernel2, 6, sizeof(cl_int), (void *)&window);

	// local and global item sizes - local_item_size could also be decided by
	// implementation
	size_t local_item_size = WORKGROUP_SIZE;
	size_t num_groups = ((gridSize - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;

#ifdef _WIN32

	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double elapsedTime;
	QueryPerformanceFrequency(&frequency); // get ticks per second
	QueryPerformanceCounter(&t1); // start timer

#elif defined __gnu_linux__

	struct timespec t1, t2;
	double elapsedTime;
	clock_gettime(CLOCK_REALTIME, &t1);

#endif

	while (iterations) {

		// Every iteration gets new seed.
		if ((iterations % 2) == 0) { // read from grid1, write to grid2
			ret = clSetKernelArg(kernel1, 5, sizeof(int), (void *)&seeds[iterations]);
			ret = clEnqueueNDRangeKernel(command_queue, kernel1, 1, NULL,
				&global_item_size, &local_item_size, 0, NULL, NULL);
		}
		else { // read from grid2, write to grid1
			ret = clSetKernelArg(kernel2, 5, sizeof(int), (void *)&seeds[iterations]);
			ret = clEnqueueNDRangeKernel(command_queue, kernel2, 1, NULL,
				&global_item_size, &local_item_size, 0, NULL, NULL);
		}

#ifdef USE_SDL

		// copy data from card when visualization is used
		if ((iterations % 2) == 0)
			ret = clEnqueueReadBuffer(command_queue, grid2, CL_TRUE, 0,
				gridSize * sizeof(unsigned char), oneDGrid, 0, NULL, NULL);
		else
			ret = clEnqueueReadBuffer(command_queue, grid1, CL_TRUE, 0,
				gridSize * sizeof(unsigned char), oneDGrid, 0, NULL, NULL);
		// render grid and display rendered content
		render1DGrid(oneDGrid, grid->width, grid->height, renderer);
		SDL_RenderPresent(renderer->SDLrenderer);

		// handle events
		while (SDL_PollEvent(&sdlEvent))
			if (sdlEvent.type == SDL_QUIT) // window's X button
				run = 0;
		if (!run)
			break;
		// delay in miliseconds
		SDL_Delay(DELAY);
#endif
		
		iterations--;
	}

	// copy results from card
	if ((ITERATIONS % 2) == 0)
		ret = clEnqueueReadBuffer(command_queue, grid2, CL_TRUE, 0,
			gridSize * sizeof(unsigned char), oneDGrid, 0, NULL, NULL);
	else
		ret = clEnqueueReadBuffer(command_queue, grid1, CL_TRUE, 0,
			gridSize * sizeof(unsigned char), oneDGrid, 0, NULL, NULL);

#ifdef _WIN32

	QueryPerformanceCounter(&t2);
	elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("%f ms\n", elapsedTime);

#elif defined __gnu_linux__
	
	clock_gettime(CLOCK_REALTIME, &t2);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsedTime += (t2.tv_nsec - t1.tv_nsec) / 1000000.0;
	printf("%f ms \n", elapsedTime);

#endif

#ifdef USE_SDL
	destroyRenderer(grid, renderer);
#endif

	// clean OpenCL
	ret = clReleaseKernel(kernel1);
	ret = clReleaseKernel(kernel2);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(grid1);
	ret = clReleaseMemObject(grid2);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	free(seeds);

	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	destroyGridFile(config);
	return 0;
}
