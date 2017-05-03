#include "thread.h"
#include "matrix.h"

int updateFoundNumber(argStruct *arguments, int found) {
	int num;
	pthread_mutex_lock(arguments->file_lock);
	if(found > *(arguments->found)) {
		*(arguments->found) = found;
		printf("updating found to %d\n", found);
	}

	num = *(arguments->found);
	pthread_mutex_unlock(arguments->file_lock);
	return num;
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
	cliques = 0;
	int received_number;
	client_struct *client_info;
	int shift = 0;
	//useful for logging purposes
	setbuf(stdout, NULL);

	client_info = (client_struct*)malloc(sizeof(client_struct));
	createClient(client_info);
	matrix = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);
	old_matrix = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);

	srand(time(NULL));
	printf("Beginning Thread %d\n", arguments->thread_id);
	for(counter_number = 130; counter_number < 1000; counter_number++) {
		printf("Trying to Solve Ramsey Number %d\n", counter_number);
		matrix_size = counter_number * counter_number;
		bzero(matrix, LARGEST_MATRIX_SIZE * sizeof(int));

		/*
		*
		*		CAN SET DIFFERENT ALGORITHMS HERE
		*
		*/
		switch(algorithm_type) {
			case RANDOM_50_50:
				initialize_50_50(matrix, counter_number);
				break;
			case RANDOM:
				copyMatrix(old_matrix, counter_number - 1, matrix, counter_number);	
				break;
			default:
				printf("\n Algorithm type not added! Please add type to thread.c switch statement around line 62! \n");
				printf("\n exiting... \n");
				exit(1);
		}
		

		while(1) {
			received_number = pollCoordinator(old_matrix, client_info);

			if(received_number >= counter_number) {
				printf("Someone has solved Ramsey Number %d, Switching to solve Counter Example %d\n", received_number, received_number + 1);
				counter_number = received_number;
				updateFoundNumber(arguments, counter_number);
				break;
			}
			if(*(arguments->found) >= counter_number) {
				counter_number = *(arguments->found);
				printf("Thread %d switching to search for %d\n", arguments->thread_id, counter_number+1);
				break;
			}
			index = getRandomIndex(counter_number);

			matrix[index] ^= 1;
			cliques = CliqueCount(matrix, counter_number);

			if(cliques == 0) {
				if(*(arguments->found) < counter_number) {
					updateFoundNumber(arguments, counter_number);
					printf("Thread %d Found Counter Example for %d!\n", arguments->thread_id, counter_number);

					received_number = sendCounterExampleToCoordinator(matrix, counter_number, old_matrix, client_info);
					
					if(received_number > counter_number) {
						counter_number = received_number;
					} else {
						sprintf(buffer, "counter_examples/counter_%d.txt", counter_number);
						if( access( buffer, F_OK ) == -1 ) {
							fp = fopen(buffer, "w");
							writeToFile(fp, matrix, counter_number, arguments);
							fclose(fp);
						}

					}
					bzero(buffer, sizeof(buffer));
				}

				break;
			} else {

				/*
				*
				*		CAN SET DIFFERENT ALGORITHMS HERE
				*
				*/
				switch(algorithm_type) {
					case RANDOM_50_50:
						flip_50_50(matrix, counter_number, matrix_size);
						break;
					case RANDOM:
						flip_random(matrix, counter_number, matrix_size);	
						break;
					default:
						printf("\n Algorithm type not added! Please add type to thread.c switch statement around line 124! \n");
						printf("\n exiting... \n");
						exit(1);
				}

			}
		}
	}

	return NULL;
}