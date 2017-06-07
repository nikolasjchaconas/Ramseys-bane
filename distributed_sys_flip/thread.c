#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "distributed_flip.h"
#include <stdlib.h>
#include "clique-count.h"
#include "client_protocol/client.h"
#include "matrix.h"
#include "limits.h"
#include "getcpuclockspeed.h"

int NUM_THREADS;
int bestCliqueCount = INT_MAX;
int bestNodeCount = 0;
long double CPU_CLOCK_SPEED;

pthread_rwlock_t bestCliqueCountMutex;

void initRandomGraph(int* graph, const int nodeCount){
	const int adjMatrixSize = nodeCount*nodeCount;
	bzero(graph, adjMatrixSize*sizeof(int));
	int edgesToPlaceInRow = ceil((nodeCount-1)/2);
	int edgesPlacedInRow = 0;
	int i;

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
	int cliqueCount;

	cliqueCount = CliqueCount(graph, nodeCount, INT_MAX);

	printf("%d %d", nodeCount, cliqueCount);

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

int sendCliqueZero(int *graph, int *nodeCount, client_struct *client_info){
	printf("T%d: Found Counter Example for %d!\n", client_info->id, *nodeCount);

	int received_number = sendCounterExampleToCoordinator(*nodeCount, 0, 0, graph, client_info);

	if(received_number > *nodeCount) {
		*nodeCount = received_number;
	}
	pthread_rwlock_rdlock(&bestCliqueCountMutex);
	bestNodeCount = *nodeCount;
	bestCliqueCount = client_info->coordinator_return->clique_count;
	pthread_rwlock_unlock(&bestCliqueCountMutex);
	return client_info->coordinator_return->clique_count;
}

int randomFlip(int *graph, int nodeCount, int cliqueCount, int *temp) {
	int index = getRandomIndex(nodeCount);
	int ret;

	while(temp[index]) index = getRandomIndex(nodeCount);

	graph[index] ^= 1;

	int newCount = CliqueCount(graph, nodeCount, cliqueCount);
	printf("Random found %d\n", newCount);
	if(newCount <= cliqueCount) {
		ret = newCount;
	} else {
		graph[index] ^= 1;
		ret = cliqueCount;
	}
	temp[index] = 1;
	return ret;
}

void *findCounterExample(void* args){
	int *graph;
	int *temp;
	int nodeCount;
	int cliqueCount;
	int index;
	int i;
	int coordinator_node_count;
	int random_explore_phase = 0;
	int permute_embedded = 0;
	int replace_me = NUM_THREADS == 1 ? 1 : 0;
	client_struct *client_info;
	coordinator_struct *coordinator_return;

	nodeCount = 150;
	client_info = (client_struct *)args;
	coordinator_return = client_info->coordinator_return;

	graph = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);
	temp = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);
	bzero(graph, LARGEST_MATRIX_SIZE);
	bzero(temp, LARGEST_MATRIX_SIZE);

	initialize_50_50(graph, nodeCount);
	cliqueCount = CliqueCount(graph, nodeCount, INT_MAX);

	printf("T%d: Beginning search on node count of %d\n", client_info->id, nodeCount);

	for(; nodeCount < 1000; ) {
		//only done when indices exhausted
		if(random_explore_phase) {
			random_explore_phase = 0;
			copyGraph(coordinator_return->out_matrix, graph, nodeCount);

			printf("T%d: Beginning Random explore phase on node count %d\n", client_info->id, nodeCount);
			//printGraph(graph, nodeCount);
			// bzero(graph, LARGEST_MATRIX_SIZE);
			// initialize_50_50(graph, nodeCount);
			cliqueCount = CliqueCount(graph, nodeCount, INT_MAX);
			printf("T%d: Original clique count is %d\n", client_info->id, cliqueCount);
			bzero(temp, LARGEST_MATRIX_SIZE);
			for(i = 0; i < nodeCount; i++) {
				// cliqueCount = randomGraphExplore(graph, nodeCount, cliqueCount);
				cliqueCount = randomFlip(graph, nodeCount, cliqueCount, temp);
				if(cliqueCount == 0) {
					printf("T%d: WOW random found a clique count of zero, bravo!\n", client_info->id);
					break;
				}
			}
		} else if (permute_embedded) {
			int try_permute_clique;
			permute_embedded = 0;
			int total_embeds = 0;

			//embed counter example into next index in graph and temp
			bzero(graph, LARGEST_MATRIX_SIZE);
			bzero(temp, LARGEST_MATRIX_SIZE);
			copyGraph(coordinator_return->out_matrix, graph, nodeCount - 1);
			copyGraph(coordinator_return->out_matrix, temp, nodeCount - 1);

			while(total_embeds != 2) {
				if(total_embeds != 0) nodeCount++;
				copyMatrix(temp, nodeCount - 1, graph, nodeCount);
				bzero(temp, LARGEST_MATRIX_SIZE);
				copyGraph(graph, temp, nodeCount);

				printf("T%d: \nTrying to embed %dx%d graph into %dx%d graph\n", client_info->id, nodeCount-1,nodeCount-1, nodeCount,nodeCount);
				permuteLastColumn(graph, nodeCount);
				cliqueCount = CliqueCount(graph, nodeCount, INT_MAX);
				printf("T%d: Embedding got %d\n", client_info->id, cliqueCount);
				for(i = 0; i < nodeCount/10; i++) {
					permuteLastColumn(temp, nodeCount);
					try_permute_clique = CliqueCount(temp, nodeCount, cliqueCount);
					printf("T%d: Embedding got %d\n", client_info->id, try_permute_clique);

					if(try_permute_clique < cliqueCount) {
						copyGraph(temp, graph, nodeCount);
						cliqueCount = try_permute_clique;
						printf("T%d: New best embedding is %d\n", client_info->id, cliqueCount);
					}

					if(cliqueCount == 0) {
						printf("T%d: WOW permuting a clique count of zero, bravo!\n", client_info->id);
						break;
					}

					// reset temp and try again
					wipeLastColumn(temp, nodeCount);
				}
				bzero(temp, LARGEST_MATRIX_SIZE);
				copyGraph(graph, temp, nodeCount);
				total_embeds++;
			}

			printf("\nT%d: Embedding ended with cliqueCount of %d\n", client_info->id, cliqueCount);
		}

		coordinator_node_count = sendCounterExampleToCoordinator(nodeCount, cliqueCount, 1, graph, client_info);
		sendCPUCycles(client_info);

		if(coordinator_node_count >= nodeCount) {
			index = coordinator_return->index;
			nodeCount = coordinator_return->counter_number;
			cliqueCount = coordinator_return->clique_count;

			pthread_rwlock_rdlock(&bestCliqueCountMutex);
			bestNodeCount = nodeCount;
			bestCliqueCount = cliqueCount;
			pthread_rwlock_unlock(&bestCliqueCountMutex);

			// if coordinator sent us zero
			if(cliqueCount == 0) {
				printf("T%d: Embedding counter example %d into %dx%d graph\n", client_info->id, nodeCount, nodeCount + 1, nodeCount + 1);
				nodeCount++;

				permute_embedded = 1;
				continue;
			} else if(index == -1) {
				cliqueCount = INT_MAX;
				//we've exhausted indeces
				random_explore_phase = 1;
				continue;
			} else {
				printf("T%d: Solving Counter Example %d at index %d\n", client_info->id, nodeCount, index);
				copyGraph(client_info->coordinator_return->out_matrix, graph, nodeCount);
			}
		} else {
			continue;
		}

		if(replace_me) {
			printf("T%d: Continuing to Replace Me\n", client_info->id);
			replaceMe(graph, nodeCount, cliqueCount, client_info);
		} else {
			printf("T%d: Continuing to Greedy Index Permute\n", client_info->id);
			threadedGreedyIndexPermute(graph, nodeCount, cliqueCount, index, client_info);
		}

		pthread_rwlock_rdlock(&bestCliqueCountMutex);
		printf("T%d: best clique count is %d and old clique count is %d\n", client_info->id, bestCliqueCount, cliqueCount);
		cliqueCount = CliqueCount(graph, nodeCount, INT_MAX);
		if(cliqueCount == 0) {
			// send example to coordinator
			int ret = sendCliqueZero(graph, &nodeCount, client_info);
			if(ret == 0) {
				permute_embedded = 1;
			}
		}
		pthread_rwlock_unlock(&bestCliqueCountMutex);

	}
	return NULL;
}

int main (int argc, char *argv[]){
	int rc;
	int t;
	int i;
	client_struct *client_info;
	NUM_THREADS = 4;
	if(argc < 2) {
		printf("Usage ./clique NUM_THREADS\n");
		exit(1);
	}
	else {
		NUM_THREADS = atoi(argv[1]);
	}
	pthread_t threads[NUM_THREADS];
	printf("\n\nYou have chosen %d Threads\n\n", NUM_THREADS);
	//useful for logging purposes
	setbuf(stdout, NULL);
	srand(time(NULL));
	pthread_rwlock_init(&bestCliqueCountMutex, NULL);

	CPU_CLOCK_SPEED = get_cpu_clock_speed();
	if(CPU_CLOCK_SPEED == 0) CPU_CLOCK_SPEED = 2600000000;

	printf("CPU CLOCK SPEED IS: %Lf\n", CPU_CLOCK_SPEED);
	for(t=0; t<NUM_THREADS; t++){
		// int nodeCount, int* graph, const int index
		printf("In main: creating thread %d\n", t);

		client_info = (client_struct*)malloc(sizeof(client_struct));
		client_info->id = t;
		client_info->coordinator_return = (coordinator_struct*)malloc(sizeof(coordinator_struct));
		createClient(client_info);

		rc = pthread_create(&threads[t], NULL, findCounterExample, (void*)client_info);
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
