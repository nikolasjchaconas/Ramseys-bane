#include <stdio.h>
#include <stdlib.h>
#include "thread.h"

#ifndef MATRIX_H
#define MATRIX_H

int get(int i, int j, int *matrix, int width);

void set(int i, int j, int *matrix, int width, int value);

int getDiagonalIndex(int width);

int getRandomIndex(int width);

//void copyMatrix(int *smaller, int smaller_width, int *larger, int larger_width);

void copyMatrix(int* oldGraph, const int oldNodeCount, int* newGraph, const int newNodeCount);

void permuteLastColumn(int* graph, const int nodeCount);

//void writeToFile(FILE *fp, int *matrix, int counter_number, argStruct *arguments);

#endif
