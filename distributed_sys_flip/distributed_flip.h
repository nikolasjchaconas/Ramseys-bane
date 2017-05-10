#ifndef DISTRIBUTED_FLIP_H
#define DISTRIBUTED_FLIP_H

//single vs multithread
#define SINGLE_THREAD 0
#define MULTI_THREAD 1

//write counter examples to file
extern int WRITE_TO_FILE;

#include <math.h>
#include <pthread.h>
#include "clique-count.h"
#include "client_protocol/client.h"
#include "greedyGraphPermute.h"

void setThreads(int threading_type);
void initialize_50_50(int *matrix, int counter_number);
void flip_random(int *matrix, int counter_number, int matrix_size);
void flip_50_50(int *matrix, int counter_number, int matrix_size);
void systematic_50_50_flip(int *matrix, int counter_number, int matrix_size, int* attempts, int* bestGraph, int* bestCount);

#endif