#include "thread.h"
#include "matrix.h"

void updateFoundNumber(argStruct *arguments, int found) {
	// pthread_mutex_lock(lock);
	*(arguments->found) = found;
	printf("updating found to %d\n", found);
	// pthread_mutex_unlock(lock);
}

void * ThreadSolve(void *arg) {
	int counter_number;
	int matrix_size;
	int cliques;
	int i;
	int *old_matrix;
	int *matrix;
	int index;
	char buffer[1024];
	FILE *fp;
	int old_size;
	int flips;
	int flip_threshold;
	argStruct *arguments = (argStruct *) arg;
	old_matrix = NULL;
	cliques = 0;

	srand(time(NULL));
	printf("Beginning Thread %d\n", arguments->thread_id);
	for(counter_number = 138; counter_number < 200; counter_number++) {
		matrix_size = counter_number * counter_number;
		matrix = (int *)malloc(sizeof(int) * matrix_size);
		bzero(matrix, matrix_size * sizeof(int));

		if(old_matrix && old_matrix != matrix) {
			copyMatrix(old_matrix, counter_number - 1, matrix, counter_number);
			free(old_matrix);
		}

		while(1) {
			if(*(arguments->found) >= counter_number) {
				counter_number = *(arguments->found);
				printf("Thread %d switching to search for %d\n", arguments->thread_id, counter_number+1);
				old_matrix = matrix;
				old_size = matrix_size;

				break;
			}
			index = getRandomIndex(counter_number);

			matrix[index] ^= 1;
			cliques = CliqueCount(matrix, counter_number);

			if(cliques == 0) {
				if(*(arguments->found) < counter_number) {
					updateFoundNumber(arguments, counter_number);
					printf("Thread %d Found Counter Example for %d!\n", arguments->thread_id, counter_number);

					sprintf(buffer, "counter_examples/counter_%d.txt", counter_number);
					
					fp = fopen(buffer, "w");
					writeToFile(fp, matrix, counter_number, arguments);
					fclose(fp);

					bzero(buffer, sizeof(buffer));
				}

				old_matrix = matrix;
				old_size = matrix_size;

				break;
			} else {
				// need to choose this better
				flip_threshold = matrix_size / 2;
				for(flips = 0; flips < flip_threshold; flips++) {
					matrix[getRandomIndex(counter_number)] ^= 1;
				}
			}
		}
	}

	return NULL;
}