#include "distributed_flip.h"
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

void systematic_50_50_flip(int *matrix, int counter_number, int matrix_size, int* attempts, int* bestGraph, int* bestCount){
	int i;
	int row;
	
	if(*attempts < 10*counter_number){
		// This is the exploratory phase where one always keep track of the best graph so far by random exploration. 
		// This is meant to function a little bit like simulated annealing.
		// The best graph after a certain number of attempts is used as the basis for a search.
		flip_50_50(matrix, counter_number, matrix_size);

		int currentCount = CliqueCount(matrix, counter_number, *bestCount);
		printf("currentCount is: %d\n",currentCount);
		if(currentCount < *bestCount){
			*bestCount = currentCount;
			for(i = 0; i < counter_number*counter_number; i++){
				bestGraph[i] = matrix[i];
			}
		}
	}
	else {
		printf("Start with new best graph!\n");
		for(i = 0; i < counter_number*counter_number; i++){
			matrix[i] = bestGraph[i];
		}
		int isNew = 0;
		//Iterate over all rows
		for(row = 0; row < counter_number-2; row++){
			//Pick the last 1 and 0 edge in the row
			int lastIndex1InRow = (row*counter_number)+(counter_number - 1);
			int lastIndex0InRow = (row*counter_number)+(counter_number - 1);
			while(matrix[lastIndex1InRow]!= 1||lastIndex1InRow<row*counter_number+row+1) lastIndex1InRow--;
			while(matrix[lastIndex0InRow]!= 0||lastIndex0InRow<row*counter_number+row+1) lastIndex0InRow--;

			//init basic vars used in the search algorithm
			int col1 = row + 1;
			int col0 = row + 1;
			int index1 = (row*counter_number)+col1;
			int index0 = (row*counter_number)+col0;
			//Flip edges until last pair of edges is flipped
			while((index1 != lastIndex1InRow+1) && (index0 != lastIndex0InRow+1)){

				//Pick the first 1 edge in the row
				index1 = (row*counter_number)+col1;
				while(matrix[index1] != 1){
					col1++;
					index1 = (row*counter_number)+col1;
				}

				//Pick the first 0 edge in the row
				index0 = (row*counter_number)+col0;
				while(matrix[index0] != 0){
					col0++;
					index0 = (row*counter_number)+col0;
				}

				//Swithc their values
				matrix[index1] = 0;
				matrix[index0] = 1;

				//Check the new matrices cliques count
				int cliqueInCopy = CliqueCount(matrix, counter_number, *bestCount);
				printf("Checking out: %d\n",cliqueInCopy);
				if(cliqueInCopy < *bestCount){
					*bestCount = cliqueInCopy;
					printf("bestCount is: %d\n",*bestCount);
					for(i = 0; i < counter_number*counter_number; i++){
						bestGraph[i] = matrix[i];
					}
					isNew = 1;
					break;
					if(*bestCount == 0) break;
				}

				//Reset matrix back to normal
				matrix[index1] = 1;
				matrix[index0] = 0;

				if(index0 == lastIndex0InRow && index1 != lastIndex1InRow){
					col1++;
					col0 = row+1;
				}
				else col0++;

				index1 = (row*counter_number)+col1;
				index0 = (row*counter_number)+col0;
			}
			if(isNew==1){
				isNew = 0;
				break; 
			}
			if(*bestCount == 0) break;
		}		
		(*attempts)++;
	}
}

// void setThreads(int threading_type) {
// 	pthread_mutex_t file_lock;
// 	int found;
// 	int index;
// 	int bound;
// 	int clique_count;
// 	pthread_t *thread_array;
// 	int error;
// 	int thread_id;
// 	int i;
// 	argStruct **arguments;
// 	argStruct *argument;

// 	pthread_mutex_init(&file_lock, NULL);
// 	found = 0;

// 	printf("---------------------------------------------\n");
// 	printf("\nBeginning Ramsey Solver.\n");
// 	printf("\nOptions Chosen:\n");
	
// 	switch(threading_type) {
// 		case SINGLE_THREAD:
// 			printf("THREADING: Single Threaded\n\n");
// 			printf("---------------------------------------------\n\n");
// 			argument = (argStruct *)malloc(sizeof(argStruct));
// 			argument->thread_id = 0;
// 			argument->found = &found;
// 			argument->file_lock = &file_lock;
// 			ThreadSolve((void*)argument);
// 			break;

// 		case MULTI_THREAD:
// 			printf("THREADING: Multi Threaded\n\n");
// 			printf("---------------------------------------------\n\n");
// 			thread_array = (pthread_t *)malloc(sizeof(pthread_t) * NUM_THREADS);
// 			arguments = (argStruct **)malloc(sizeof(argStruct*) * NUM_THREADS);

// 			for(thread_id = 0; thread_id < NUM_THREADS; thread_id++) {
// 				arguments[thread_id] = (argStruct *)malloc(sizeof(argStruct));
// 				arguments[thread_id]->thread_id = thread_id;
// 				arguments[thread_id]->found = &found;
// 				arguments[thread_id]->file_lock = &file_lock;
// 				error = pthread_create(&(thread_array[thread_id]), NULL, ThreadSolve, (void *)arguments[thread_id]);
// 				if(error) {
// 					fprintf(stderr, "error creating thread\n");
// 					exit(1);
// 				}
// 			}

// 			//join threads
// 			for(i=0; i < NUM_THREADS; i++) {
// 				error = pthread_join(thread_array[i],NULL);
// 				if (error != 0) {
// 					fprintf(stderr, "error joining thread\n");
// 				}

// 			}
// 			break;

// 		default:
// 			printf("You did not specify a valid threading type.\n");
// 			printf("Exiting...\n");
// 			exit(1);
// 	}
// }