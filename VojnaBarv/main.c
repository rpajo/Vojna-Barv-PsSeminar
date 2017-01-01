#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "grid.h"
#include "file.h"
#include "pcg_basic.h"

#include "CL\cl.h"

//#define USE_SDL // use SDL - comment to not use

#ifdef USE_SDL
#include <SDL.h>
#include "render.h"
#define WINDOW_NAME		"Vojna Barv"
#define DELAY			((unsigned int) 100)
#endif

#define ITERATIONS		30000
#define WINDOW			2
#define FILE_NAME		"grid3.txt"
#define NTHREADS		2

#define SIZE			1024
#define WORKGROUP_SIZE	1024
#define MAX_SOURCE_SIZE 32768

pcg32_random_t rngs[NTHREADS];
int nthreads = NTHREADS;

int main(int argc, char **argv) {

	// Initialize grid 
	GridFile *config = parseFile(FILE_NAME);
	if (config == NULL) return 1;
	Grid *grid = config->initialGrid;
	Grid *tempGrid = createGrid(grid->height, grid->width);
	if (tempGrid == NULL) printf("main error: cannot allocate space for temporary grid.\n");

	config->initialGrid = NULL;
	destroyGridFile(config);


	// OpenCL setup ------------------------------------------------------------------------------------------------------------------------------------------------

	// Open kernel
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("kernel.cl", "r");
	if (!fp)
	{
		fprintf(stderr, ":-(#\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);

	// get platform info
	cl_int ret;
	cl_platform_id platform_id[2];
	cl_uint			ret_num_platforms;
	ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms); // max. število platform, kazalec na platforme, dejansko število platform

	// get device info
	cl_device_id	device_id[2];
	cl_uint			ret_num_devices;
	ret = clGetDeviceIDs(platform_id[1], CL_DEVICE_TYPE_GPU, 2, device_id, &ret_num_devices);	// izbrana platforma, tip naprave, koliko naprav nas zanima
																								// kazalec na naprave, dejansko število naprav

	// context
	cl_context context = clCreateContext(NULL, 1, &device_id[0], NULL, NULL, &ret);				// kontekst: vkljuèene platforme - NULL je privzeta, število naprav, 
																								// kazalci na naprave, kazalec na call-back funkcijo v primeru napake
																								// dodatni parametri funkcije, številka napake
	// ommand Queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);		// kontekst, naprava, INORDER/OUTOFORDER, napake
	
	// Devide workload
	size_t local_item_size = WORKGROUP_SIZE;
	size_t num_groups = ((grid->height * grid->height) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
		
	// Mermory allocation
	cl_mem device_memory_grid = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (grid->height * grid->height)*sizeof(unsigned char), grid->colors, &ret);
	cl_mem device_memory_tempGrid = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (grid->height * grid->height)*sizeof(unsigned char), tempGrid->colors, &ret);
	// kontekst, naèin, koliko, lokacija na hostu, napaka

	// Prepare program
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, NULL, &ret);
	// Compile
	ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);	// program, število naprav, lista naprav, opcije pri prevajanju,
																		// kazalec na funkcijo, uporabniški argumenti
	// Log
	size_t build_log_len;
	char *build_log;
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);	// program, naprava, tip izpisa, maksimalna dolžina niza, 
																										// kazalec na niz, dejanska dolžina niza
	build_log = (char *)malloc(sizeof(char)*(build_log_len + 1));
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, build_log_len, build_log, NULL);
	printf("%s\n", build_log);
	free(build_log);

	// Prepare kernel
	cl_kernel kernel = clCreateKernel(program, "processGrid", &ret); // program, ime šèepca, napaka

#ifdef USE_SDL
	Renderer *renderer = initRenderer(WINDOW_NAME, grid, config->cellSize);
	if (renderer == NULL || createTexturesFromColors(config, renderer) != 0 || tempGrid == NULL) {
		destroyRenderer(grid, renderer);
		destroyGrid(grid);
		destroyGrid(tempGrid);
		destroyGridFile(config);
		return 1;
	}
#endif

	unsigned int iterations = ITERATIONS;

#ifdef USE_SDL
	SDL_Event sdlEvent;
	int run = 1;
#endif

	double startTime = omp_get_wtime();

	while (iterations--) {
		
		//processGrid(grid, tempGrid, WINDOW);
		
		// kernel arguments
		ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)grid);
		ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&tempGrid);

		// kernel start
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
		// vrsta, šèepec, dimenzionalnost, mora biti NULL, kazalec na število vseh niti, kazalec na lokalno število niti, 
		// dogodki, ki se morajo zgoditi pred klicem

		// kernel arguments -> switch grids and start new kernel
		ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)tempGrid);
		ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)grid);
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);


#ifdef USE_SDL
		// render grid and display rendered content
		renderGrid(grid, renderer);
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

	}

	// Copy result
	/*
	ret = clEnqueueReadBuffer(command_queue, gird, CL_TRUE, 0, (grid->height * grid->height)*sizeof(unsigned char), grid, 0, NULL, NULL);
	// branje v pomnilnik iz naparave, 0 = offset
	// zadnji trije - dogodki, ki se morajo zgoditi prej
	*/
	double endTime = omp_get_wtime();
	printf("%f s\n", endTime-startTime);

#ifdef USE_SDL
	destroyRenderer(grid, renderer);
#endif

	// Free memory
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(device_memory_grid);
	ret = clReleaseMemObject(device_memory_tempGrid);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	//destroyGrid(grid);
	//destroyGrid(tempGrid);
	return 0;
}
