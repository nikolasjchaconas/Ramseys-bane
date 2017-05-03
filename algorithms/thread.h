#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "algorithms.h"

void *ThreadSolve(void *arg);

typedef struct arg {
	int thread_id;
	int *found;
	pthread_mutex_t *file_lock;
} argStruct;

#endif