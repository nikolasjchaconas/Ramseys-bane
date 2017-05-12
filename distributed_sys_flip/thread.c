#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "distributed_flip.h"
#include <stdlib.h>
#include "clique-count.h"
#include "client_protocol/client.h"
#include "matrix.h"
#define NUM_THREADS	2

int bestCliqueCount;
pthread_rwlock_t bestCliqueCountMutex;
client_struct *client_info;
char buffer[1024];

void initRandomGraph(int* graph, const int nodeCount){
	const int adjMatrixSize = nodeCount*nodeCount;
	bzero(graph, adjMatrixSize*sizeof(int));
	int edgesToPlaceInRow = ceil((nodeCount-1)/2);
	int edgesPlacedInRow = 0;
	int i;
	int row;

	int edgesInPreviousRow = 0;

	for(i = 0; i < nodeCount-1; i++){
		int edgeValue = 1;
		while(1){
			int randRow = i;
			int randCol = (rand() % (nodeCount-randRow-1))+randRow+1;
			edgeValue = graph[(randRow*nodeCount)+randCol];
			if(edgeValue==0){
			graph[(randRow*nodeCount)+randCol] = 1;
			edgesPlacedInRow++;
			if(edgesToPlaceInRow == edgesPlacedInRow){
				edgesPlacedInRow = 0;
				edgesToPlaceInRow = ceil((nodeCount - 1 - i)/2);
				break;
			}
			}
		}
	}
}

void printGraph(int *graph, const int nodeCount){
	int adjMatrixSize;
	int i;
	int j;
	int value;

	printf("%d 0", nodeCount);

	adjMatrixSize = nodeCount * nodeCount;
	
	for(i = 0; i < nodeCount; i++) {
	printf("\n");
	for(j = 0; j < nodeCount; j++) {
		value = get(i, j, graph, nodeCount);
		printf("%d ", value);
	}
	}
}

void initIndexQueue(int* indexQueue, int* graph, const int indexQueueSize, const int adjMatrixSize){
	bzero(indexQueue, indexQueueSize*sizeof(int)); 
	int index;
	int indexQueueNr = 0;
	for(index = 0; index < adjMatrixSize; index++){
	if(graph[index] == 1){
		indexQueue[indexQueueNr] = index;
		indexQueueNr++;
	}
	}
}

void *findCounterExample(void* args){
	int *graph;
	int nodeCount;
	int cliqueCount;
	int index;
	int rc;
	long t;
	int i;
	int random_iterations;
	int coordinator_node_count;
	int received_number;


	random_iterations = 1;
	nodeCount = 160;
	coordinator_struct *coordinator_return;
	coordinator_return = (coordinator_struct *)args;

	graph = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);

	for(; nodeCount < 1000; nodeCount++) {
		bzero(graph, LARGEST_MATRIX_SIZE);
		initialize_50_50(graph, nodeCount);
		cliqueCount = CliqueCount(graph, nodeCount);
		for(i = 0; i < random_iterations; i++) {
			cliqueCount = randomGraphExplore(graph, nodeCount, cliqueCount);	
		}

		coordinator_node_count = sendCounterExampleToCoordinator(nodeCount, cliqueCount, -1, graph, client_info);

		if(coordinator_node_count >= nodeCount) {
			printf("Someone has solved Ramsey Number %d, Switching to solve Counter Example %d\n", nodeCount, coordinator_node_count + 1);
			copyGraph(client_info->coordinator_return->out_matrix, graph, nodeCount);
			nodeCount = coordinator_return->counter_number;
			cliqueCount = coordinator_return->clique_count;
			index = coordinator_return->index;	
		} else {
			nodeCount--;
			continue;
		}
		
		threadedGreedyIndexPermute(graph, nodeCount, cliqueCount, index);

		pthread_rwlock_rdlock(&bestCliqueCountMutex);
		if(bestCliqueCount == 0) {
			printf("Found Counter Example for %d!\n", nodeCount);

			received_number = sendCounterExampleToCoordinator(nodeCount, 0, -1, graph, client_info);

			if(received_number > nodeCount) {
				nodeCount = received_number - 1;
			}
		} else {
			if(bestCliqueCount > cliqueCount) {
				received_number = sendCounterExampleToCoordinator(nodeCount, 0, -1, graph, client_info);
				if(received_number > nodeCount) {
					nodeCount = received_number;
				}
			}
			pthread_rwlock_unlock(&bestCliqueCountMutex);
			nodeCount--;
			continue;
		}
		pthread_rwlock_unlock(&bestCliqueCountMutex);

	}
	return NULL;
}

int main (int argc, char *argv[]){
	int nodeCount;
	int rc;
	int t;
	int i;
	pthread_t threads[NUM_THREADS];

	srand(time(NULL));
	client_info = (client_struct*)malloc(sizeof(client_struct));
	client_info->coordinator_return = (coordinator_struct*)malloc(sizeof(coordinator_struct));
	createClient(client_info);

	pthread_rwlock_init(&bestCliqueCountMutex, NULL);

	for(t=0; t<NUM_THREADS; t++){
		// int nodeCount, int* graph, const int index
		printf("In main: creating thread %d\n", t);

		rc = pthread_create(&threads[t], NULL, findCounterExample, (void*)client_info->coordinator_return);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for(i=0; i < NUM_THREADS; i++) {
		int error = pthread_join(threads[i],NULL);
		if (error != 0) {
			fprintf(stderr, "error joining thread\n");
		}
	}

	/* Last thing that main() should do */
	pthread_exit(NULL);
	return 0;
}
