#include "thread.h"
#include "matrix.h"
#include <math.h>

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
	int *matrix = NULL;
	int index;
	char buffer[1024];
	FILE *fp;
	int flips;
	int flip_threshold;
	argStruct *arguments = (argStruct *) arg;
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
		free(matrix);
		matrix = (int *)malloc(sizeof(int) * matrix_size);
		bzero(matrix, matrix_size * sizeof(int));

		int edgesToPlace = (counter_number*(counter_number-1))/4;
		int rowToPlace = 0;
		int edgesToPlaceInRow = ceil((counter_number-1)/2);
		int edgesPlacedInRow = 0;
		
	    for(i = 0; i < counter_number-1; i++){
		    int edgeValue = 1;
		    while(1){
			    int randRow = i;
			    int randCol = (rand() %  (counter_number-randRow-1))+randRow+1;
			    edgeValue = matrix[(randRow*counter_number)+randCol];
			    if(edgeValue==0){
			    	matrix[(randRow*counter_number)+randCol] = 1;
			    	edgesPlacedInRow++;
			    	if(edgesToPlaceInRow == edgesPlacedInRow){
		    			edgesPlacedInRow = 0;
		    			edgesToPlaceInRow = ceil((counter_number - 1 - i)/2);
			    		break;
			    	}
			    }
		    }
	    }

		while(1) {
			received_number = pollCoordinator(tmp, client_info);

			if(received_number >= counter_number) {
				printf("Someone has solved Ramsey Number %d, Switching to solve Counter Example %d\n", received_number, received_number + 1);
				printf("freeing matrix\n");
				free(tmp);
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

					received_number = sendCounterExampleToCoordinator(matrix, counter_number, tmp, client_info);
					if(received_number > counter_number) {
						printf("freeing matrix here\n");
						free(tmp);
						counter_number = received_number;
					}

				}


				break;
			} else {
				// need to choose this better
				int flip_threshold = matrix_size*0.2;
				for(i = 0; i < flip_threshold; i++){
       				
       				int randRow = rand() % (counter_number-2);
        			int randCol = (rand() %  (counter_number-randRow-1))+randRow+1;
					int index1 = (randRow*counter_number)+randCol;

					int randRow2 = randRow;//rand() % (counter_number-2);
        			int randCol2 = (rand() %  (counter_number-randRow2-1))+randRow2+1;
        			int index2 = (randRow2*counter_number)+randCol2;

        			if((matrix[index1] ^ matrix[index2]) == 1){
        				matrix[index1] = !matrix[index1];
        				matrix[index2] = !matrix[index2];
        			}

    			}

    			//writeToFile(stdout, matrix, counter_number, arguments);

			}
		}
	}

	return NULL;
}
