#ifndef ALGORITHMS_H
#define ALGORITHMS_H

//single vs multithread
#define SINGLE_THREAD 0
#define MULTI_THREAD 1

//algorithms
/*

	Want to add a new algorithm?

	STEP 1
		- Add new algorithm macro here so that it can be called from main.

	STEP 2
		- add your algorithm helper function definitions here, and their implementations
		in algorithms.c
	
	STEP 3
		-update the case switch statements in:
			--algorithms.c: case switch statement for printing what algorithm is being used
			--thread.c: case switch statement for using your algorithm helper functions.

	STEP 4
		- specify in main.c that you would like to use that algorithm.

*/

#define RANDOM 2
#define RANDOM_50_50 3

//write counter examples to file
extern int WRITE_TO_FILE;

extern int ALGORITHM_TYPE;

#include <math.h>
#include <pthread.h>
#include "clique-count.h"
#include "client_protocol/client.h"

void setThreads(int threading_type);
void initialize_50_50(int *matrix, int counter_number);
void flip_random(int *matrix, int counter_number, int matrix_size);
void flip_50_50(int *matrix, int counter_number, int matrix_size);

#endif