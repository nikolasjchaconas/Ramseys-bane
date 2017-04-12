#include "clique-count.h"
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>


int get(int i, int j, int *matrix, int width) {
	return(matrix[i * width + j]);
}

void set(int i, int j, int *matrix, int width, int value) {
	matrix[i * width + j] = value;
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

void writeToFile(FILE *fp, int *matrix, int counter_number) {
	int matrix_size;
	int i;
	int j;
	int value;

	fprintf(fp, "%d 0", counter_number);

	matrix_size = counter_number * counter_number;
	
	for(i = 0; i < counter_number; i++) {
		fwrite("\n", 1, 1, fp);
		for(j = 0; j < counter_number; j++) {
			value = get(i, j, matrix, counter_number);
			fprintf(fp, "%d ", value);
		}
	}
}

int main() {
	int counter_number;
	int matrix_size;
	int cliques;
	int i;
	int *old_matrix;
	int *matrix;
	FILE *fp;
	char buffer[1024];
	int rand_col;
	int rand_row;
	int rand_index;
	int old_size;

	old_matrix = NULL;
	cliques = 0;
	srand(time(NULL));
	
	for(counter_number = 10; counter_number < 100; counter_number++) {
		matrix_size = counter_number * counter_number;
		matrix = (int *)malloc(sizeof(int) * matrix_size);
		bzero(matrix, matrix_size * sizeof(int));

		if(old_matrix && old_matrix != matrix) {
			copyMatrix(old_matrix, counter_number - 1, matrix, counter_number);
			free(old_matrix);
		}

		while(1) {
			rand_col = (double)rand()/RAND_MAX * (counter_number - 1) + 1;
			rand_row = (double)rand()/RAND_MAX * rand_col;
			rand_index = rand_row * counter_number + rand_col;

			// printf("changing (%d, %d) => index: %d\n", rand_col, rand_row, rand_index);
			matrix[rand_index] ^= 1;
			cliques = CliqueCount(matrix, counter_number);

			if(cliques == 0) {
				printf("Found Counter Example for %d!\n", counter_number);

				sprintf(buffer, "counter_examples/counter_%d.txt", counter_number);
				
				fp = fopen(buffer, "w");
				writeToFile(fp, matrix, counter_number);
				fclose(fp);

				bzero(buffer, sizeof(buffer));
				old_matrix = matrix;
				old_size = matrix_size;
				break;
			}
		}
	}	

	return 0;
}