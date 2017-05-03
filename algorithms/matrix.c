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

void copyMatrix(int *smaller, int smaller_width, int *larger, int larger_width) {
	int start;
	int value;
	int i;
	int j;

	start = 2;
	for(i = 0; i < larger_width - 2; i++) {
		for(j = start; j < larger_width; j++) {
			value = get(i, j - 2, smaller, smaller_width);
			set(i, j, larger, larger_width, value);
		}
		start++;
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