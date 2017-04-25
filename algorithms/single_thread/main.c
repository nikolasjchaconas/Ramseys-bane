#include "clique-count.h"
#include "../client_protocol/client.h"
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>

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
	int index;
	int bound;
	int clique_count;
	int flips;
	int flip_threshold;
	int *tmp;
	int received_number;
	tmp = NULL;
	old_matrix = NULL;

	counter_number = sendCounterExampleToCoordinator(NULL, 0, old_matrix) + 1;
	if(counter_number >= 0) {
		printf("Received counter number of %d from the server\n", counter_number - 1);	
	} else {
		printf("Sujaya there was an error!!!\n");
	}

	clique_count = INT_MAX;
	cliques = 0;
	srand(time(NULL));
	
	for(; counter_number < 1000; counter_number++) {
		printf("Trying to solve Ramsey Number %d\n", counter_number);
		matrix_size = counter_number * counter_number;
		
		matrix = (int *)malloc(sizeof(int) * matrix_size);
		bzero(matrix, matrix_size * sizeof(int));
		
		// bound = 1 << (counter_number - 1);
		// bound = 1;

		if(old_matrix && old_matrix != matrix) {
			copyMatrix(old_matrix, counter_number - 1, matrix, counter_number);
			free(old_matrix);
		}

		while(1) {
			// if (flip_diagonal) {
			// 	// randomly flip diagonal pieces
			// 	if(flip_diagonal > bound) {
			// 		flip_diagonal = 0;
			// 	} else {
			// 		index = getDiagonalIndex(counter_number);

			// 		flip_diagonal++;
			// 	}
			// }
			// else {
				// randomly flip bits
			received_number = pollCoordinator(NULL, 0, tmp);
			if(received_number >= counter_number) {
				printf("Someone has solved Ramsey Number %d, Switching to solve Counter Example %d\n", received_number, received_number + 1);
				free(matrix);
				matrix = tmp;
				old_matrix = tmp;
				counter_number = received_number;
				break;
			}

			index = getRandomIndex(counter_number);
			// }

			matrix[index] ^= 1;
			cliques = CliqueCount(matrix, counter_number);

			if(cliques == 0) {
				printf("Found Counter Example for %d!\n", counter_number);
				received_number = sendCounterExampleToCoordinator(matrix, counter_number, tmp);
				if(received_number > counter_number) {
					free(matrix);
					matrix = tmp;
					counter_number = received_number;
				}

				//sprintf(buffer, "counter_examples/counter_%d.txt", counter_number);
				
				// fp = fopen(buffer, "w");
				// writeToFile(fp, matrix, counter_number);
				// fclose(fp);

				bzero(buffer, sizeof(buffer));
				old_matrix = matrix;
				break;

			} else {
				flip_threshold = 2000;
				for(flips = 0; flips < flip_threshold; flips++) {
					matrix[getRandomIndex(counter_number)] ^= 1;
				}
			}
		}
	}	

	return 0;
}