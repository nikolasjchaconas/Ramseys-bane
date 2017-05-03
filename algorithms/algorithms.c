#include "algorithms.h"
#include "thread.h"
#include "matrix.h"

//4 threads is optimal
#define NUM_THREADS 4

void initialize_50_50(int *matrix, int counter_number) {
	int edgesToPlace = (counter_number*(counter_number-1))/4;
	int rowToPlace = 0;
	int edgesToPlaceInRow = ceil((counter_number-1)/2);
	int edgesPlacedInRow = 0;
	int i;

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
}

void flip_random(int *matrix, int counter_number, int matrix_size) {
	// need to choose this better
	int flip_threshold = matrix_size / 4;
	int flips;

	for(flips = 0; flips < flip_threshold; flips++) {
		matrix[getRandomIndex(counter_number)] ^= 1;
	}
}

void flip_50_50(int *matrix, int counter_number, int matrix_size) {
	int flip_threshold = matrix_size*0.2;
	int i;

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
}


void setThreads(int threading_type) {
	pthread_mutex_t file_lock;
	int found;
	int index;
	int bound;
	int clique_count;
	pthread_t *thread_array;
	int error;
	int thread_id;
	int i;
	argStruct **arguments;
	argStruct *argument;

	pthread_mutex_init(&file_lock, NULL);
	found = 0;

	printf("---------------------------------------------\n");
	printf("\nBeginning Ramsey Solver.\n");
	printf("\nOptions Chosen:\n");

	switch(ALGORITHM_TYPE) {
		case RANDOM_50_50:
			printf("\nALGORITHM: Bjarte Random 50 50 graph algorithm\n");
			break;
		case RANDOM:
			printf("\nALGORITHM: Nik Random Algorithm\n");	
			break;
		default:
			printf("\nAlgorithm type not added! Please add type to algorithms.c switch statement!\n");
	}
	
	switch(threading_type) {
		case SINGLE_THREAD:
			printf("THREADING: Single Threaded\n\n");
			printf("---------------------------------------------\n\n");
			argument = (argStruct *)malloc(sizeof(argStruct));
			argument->thread_id = 0;
			argument->found = &found;
			argument->file_lock = &file_lock;
			ThreadSolve((void*)argument);
			break;

		case MULTI_THREAD:
			printf("THREADING: Multi Threaded\n\n");
			printf("---------------------------------------------\n\n");
			thread_array = (pthread_t *)malloc(sizeof(pthread_t) * NUM_THREADS);
			arguments = (argStruct **)malloc(sizeof(argStruct*) * NUM_THREADS);

			for(thread_id = 0; thread_id < NUM_THREADS; thread_id++) {
				arguments[thread_id] = (argStruct *)malloc(sizeof(argStruct));
				arguments[thread_id]->thread_id = thread_id;
				arguments[thread_id]->found = &found;
				arguments[thread_id]->file_lock = &file_lock;
				error = pthread_create(&(thread_array[thread_id]), NULL, ThreadSolve, (void *)arguments[thread_id]);
				if(error) {
					fprintf(stderr, "error creating thread\n");
					exit(1);
				}
			}

			//join threads
			for(i=0; i < NUM_THREADS; i++) {
				error = pthread_join(thread_array[i],NULL);
				if (error != 0) {
					fprintf(stderr, "error joining thread\n");
				}

			}
			break;

		default:
			printf("You did not specify a valid threading type.\n");
			printf("Exiting...\n");
			exit(1);
	}


}