#include "matrix.h"

int get(int i, int j, int *matrix, int width) {
	return(matrix[i * width + j]);
}

void set(int i, int j, int *matrix, int width, int value) {
	matrix[i * width + j] = value;
}

int getDiagonalIndex(int width) {
	int rand_col;
	int rand_row;
	rand_col = (double)rand()/RAND_MAX * (width - 1) + 1;
	rand_row = rand_col - 1;
	return rand_row * width + rand_col;
}

int getRandomIndex(int width) {
	int rand_col;
	int rand_row;

	rand_col = (double)rand()/RAND_MAX * (width - 1) + 1;
	rand_row = (double)rand()/RAND_MAX * rand_col;
	return rand_row * width + rand_col;
}

void permuteLastColumn(int* graph, const int nodeCount){
    const int totalTypeOneEdgesToPlace = floor((nodeCount - 1) / 2);
    int edgesPlaced = 0;
    while(edgesPlaced < totalTypeOneEdgesToPlace){
        const int randRow = rand() % (nodeCount);
        const int index = (randRow * nodeCount) - 1;
        if(graph[index] == 0){
            graph[index] = 1;
            edgesPlaced++;
        }
    }
}

void copyMatrix(int* oldGraph, const int oldNodeCount, int* newGraph, const int newNodeCount){
    int row;
    int col;
    int i;
    //Set new graph to all zeros
    for(i = 0; i < newNodeCount*newNodeCount; i++){
        newGraph[i] = 0;
    }

    //Copy over the old matrix
    for(row = 0; row < oldNodeCount; row++){
        for(col = 0; col < oldNodeCount; col++){
            int index = row * oldNodeCount + col;
            int index2 = row * newNodeCount + col;
            newGraph[index2] = oldGraph[index];
        }
    }
}

void writeToFile(FILE *fp, int *matrix, int counter_number, argStruct *arguments) {
	int matrix_size;
	int i;
	int j;
	int value;

	pthread_mutex_lock(arguments->file_lock);
	fprintf(fp, "%d 0", counter_number);

	matrix_size = counter_number * counter_number;

	for(i = 0; i < counter_number; i++) {
		fwrite("\n", 1, 1, fp);
		for(j = 0; j < counter_number; j++) {
			value = get(i, j, matrix, counter_number);
			fprintf(fp, "%d ", value);
		}
	}
	pthread_mutex_unlock(arguments->file_lock);
}
