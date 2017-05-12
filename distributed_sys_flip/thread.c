#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "greedyGraphPermute.h"
#include <stdlib.h>
#include "clique-count.h"
#include "client_protocol/client.h"
#include "matrix.h"
#define NUM_THREADS	100

int bestCliqueCount;
pthread_rwlock_t bestCliqueCountMutex;
client_struct *client_info;
int* old_matrix;
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

void findCounterExample(int nodeCount){

  // int coordinatorValue = pollCoordinator(old_matrix, client_info);
  // if(coordinatorValue >= nodeCount) {
  //   printf("Someone has solved Ramsey Number %d, Switching to solve Counter Example %d\n", nodeCount, coordinatorValue+1);
  //   nodeCount = coordinatorValue+1;
  // }

  const int adjMatrixSize = nodeCount*nodeCount;
  int graph[adjMatrixSize];
  initRandomGraph(graph, nodeCount);
  bestCliqueCount = CliqueCount(graph, nodeCount);

  int i;
  for(i = 0; i < 100; i++){
    bestCliqueCount = randomGraphExplore(graph, nodeCount, bestCliqueCount);
  }

  int indexQueueSize = ceil(adjMatrixSize/4);
  int indexQueue[indexQueueSize];
  int nextIndex = 0;
  initIndexQueue(indexQueue, graph, indexQueueSize, adjMatrixSize);

  pthread_t threads[NUM_THREADS];
  int rc;
  long t;
  struct params** readParams;
  readParams = (struct params**) malloc(NUM_THREADS * sizeof(struct params*));
  for(i = 0; i < NUM_THREADS; i++){
    readParams[i] = (struct params*) malloc(sizeof(struct params));
  }
  while(bestCliqueCount > 0){
    for(t=0; t<NUM_THREADS; t++){
     printf("In main: creating thread %ld\n", t);
     readParams[t]->graph = graph;
     readParams[t]->nodeCount = nodeCount;
     readParams[t]->cliqueCount = bestCliqueCount;
     readParams[t]->index = indexQueue[nextIndex = ((nextIndex+500)%nodeCount)];

     rc = pthread_create(&threads[t], NULL, threadedGreedyIndexPermute, readParams[t]);
     if (rc){
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
  }
  for(i = 0; i < NUM_THREADS; i++){
    free(readParams[i]);
  }
  free(readParams);

  // if(bestCliqueCount == 0) {
  //   printf("Found Counter Example for %d!\n", nodeCount);

  //   int received_number = sendCounterExampleToCoordinator(graph, nodeCount, old_matrix, client_info);
  //   FILE *fp;
  //   if(received_number > nodeCount) {
  //     nodeCount = received_number;
  //   } else {
  //     if(1) {
  //       sprintf(buffer, "counter_examples/counter_%d.txt", nodeCount);
  //       if( access( buffer, F_OK ) == -1 ) {
  //         fp = fopen(buffer, "w");
  //         writeToFile(fp, graph, nodeCount);
  //         fclose(fp);
  //       }
  //     }
  //     bzero(buffer, sizeof(buffer));
  //   }
  // }

}

int main (int argc, char *argv[]){
  srand(time(NULL));
  old_matrix = (int *)malloc(sizeof(int) * LARGEST_MATRIX_SIZE);
  client_info = (client_struct*)malloc(sizeof(client_struct));
  //createClient(client_info);
  pthread_rwlock_init(&bestCliqueCountMutex, NULL);
  int nodeCount;
  int coordinatorValue = 160;//pollCoordinator(old_matrix, client_info);

  for(nodeCount = coordinatorValue+1; nodeCount < 810; nodeCount++){
    findCounterExample(nodeCount);
  }

  /* Last thing that main() should do */
  pthread_exit(NULL);
  return 0;
}
