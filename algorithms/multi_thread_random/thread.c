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
	int flips;
	int flip_threshold;
	argStruct *arguments = (argStruct *) arg;
	old_matrix = NULL;
	cliques = 0;
	int received_number;
	int *tmp;
	client_struct *client_info;

	client_info = (client_struct*)malloc(sizeof(client_struct));
	createClient(client_info);

	srand(time(NULL));
	printf("Beginning Thread %d\n", arguments->thread_id);
	for(counter_number = 130; counter_number < 1000; counter_number++) {
		printf("Trying to Solve Ramsey Number %d\n", counter_number);
		matrix_size = counter_number * counter_number;
		matrix = (int *)malloc(sizeof(int) * matrix_size);
		bzero(matrix, matrix_size * sizeof(int));

		if(old_matrix && old_matrix != matrix) {
			copyMatrix(old_matrix, counter_number - 1, matrix, counter_number);
			printf("freeing old matrix\n");
			free(old_matrix);
			old_matrix = NULL;
		}

		while(1) {
			received_number = pollCoordinator(tmp, client_info);

			if(received_number >= counter_number) {
				printf("Someone has solved Ramsey Number %d, Switching to solve Counter Example %d\n", received_number, received_number + 1);
				printf("freeing matrix\n");
				free(matrix);
				matrix = tmp;
				old_matrix = tmp;
				counter_number = received_number;
				updateFoundNumber(arguments, counter_number);
				break;
			}
			if(*(arguments->found) >= counter_number) {
				counter_number = *(arguments->found);
				printf("Thread %d switching to search for %d\n", arguments->thread_id, counter_number+1);
				old_matrix = matrix;
				break;
			}
			index = getRandomIndex(counter_number);

			matrix[index] ^= 1;
			cliques = CliqueCount(matrix, counter_number);

			if(cliques == 0) {
				if(*(arguments->found) < counter_number) {
					updateFoundNumber(arguments, counter_number);
					printf("Thread %d Found Counter Example for %d!\n", arguments->thread_id, counter_number);

					received_number = sendCounterExampleToCoordinator(matrix, counter_number, tmp, client_info);
					if(received_number > counter_number) {
						printf("freeing matrix here\n");
						free(matrix);
						matrix = tmp;
						counter_number = received_number;
					}

					// sprintf(buffer, "counter_examples/counter_%d.txt", counter_number);
					
					// fp = fopen(buffer, "w");
					// writeToFile(fp, matrix, counter_number, arguments);
					// fclose(fp);

					// bzero(buffer, sizeof(buffer));
				}

				old_matrix = matrix;

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
