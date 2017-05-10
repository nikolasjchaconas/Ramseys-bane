#include <math.h>
#include <stdio.h>
#include <stdlib.h>



//Greedy search methods
const int getNextOneIndexInRow(int* graph, const int nodeCount, const int row, int* col);

const int getNextZeroIndexInRow(int* graph, const int nodeCount, const int row, int* col);

const int getLastZeroIndexInRow(int* graph, const int nodeCount, const int row);

const int getLastOneIndexInRow(int* graph, const int nodeCount, const int row);

void switchEdges(int* graph, const int index1, const int index0);

const int greedyIndexPermute(int* graph, const int nodeCount, const int cliqueCount, const int index);

const int greedyRowPermute(int* graph, const int nodeCount, const int cliqueCount, const int row);

//Main algorithm
const int greedyGraphPermute(int* graph, const int nodeCount, const int cliqueCount);


//Exploration related methods
void randomGraphPermute(int* graph, const int nodeCount, const int permuteFactor);

void copyGraph(int* graph, int* copy, const int nodeCount);

const int randomGraphExplore(int* graph, const int nodeCount, const int cliqueCount);