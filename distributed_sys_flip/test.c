#include <stdio.h>
#include <strings.h>
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //for exit(0);
#include <sys/socket.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include "clique-count.h"
#include <limits.h>
#include <math.h>

int get(int i, int j, int *matrix, int width) {
	return(matrix[i * width + j]);
}

void set(int i, int j, int *matrix, int width, int value) {
	matrix[i * width + j] = value;
}

int hostname_to_ip(char *hostname , char* ip)
{
	struct hostent *host_struct;
	struct in_addr **addr_list;
	int i;
	i = 0;
	host_struct = gethostbyname(hostname);

	if (host_struct == NULL) 
	{
		// get the host info
		perror("gethostname:");
		return -1;
	}
 
	addr_list = (struct in_addr **) host_struct->h_addr_list;
	 
	while(addr_list[i] != NULL) 
	{
		printf("IP is %s\n", inet_ntoa(*addr_list[i++]));
	}

	return 1;
}

void permuteLastColumn(int* graph, const int nodeCount){
    const int totalTypeOneEdgesToPlace = floor((nodeCount - 1) / 2);
    int edgesPlaced = 0;
    while(edgesPlaced < totalTypeOneEdgesToPlace){
        const int randRow = rand() % (nodeCount);
        const int index = (randRow * nodeCount) - 1;
        if(graph[index] == 0){
            graph[index] = 1;
            edgesPlaced++;
        }
    }
}

void iterateLastColumn(int* graph, const int nodeCount, int oldCount){
    const int totalTypeOneEdgesToPlace = 10000;
    int edgesPlaced = 0;
    int newCount;
    int randRow;
    for(randRow = 0; edgesPlaced < 10;){
        const int index = (randRow * nodeCount) - 1;

        graph[index] ^= 1;
        newCount = CliqueCount(graph, nodeCount, oldCount);
        printf("Flipping row %d made count %d\n",randRow, newCount);
        if(newCount < oldCount) {
        	printf("good move here! at %d count\n", newCount);
        	oldCount = newCount;
			edgesPlaced++;
        } else {
        	graph[index] ^= 1;
        }
        randRow = (randRow + 1)%nodeCount;
    }
}

void randomizeLastColumn(int* graph, const int nodeCount, int oldCount){
    const int totalTypeOneEdgesToPlace = 10000;
    int edgesPlaced = 0;
    int newCount;
    while(edgesPlaced < 10){
        const int randRow = rand() % (nodeCount);
        const int index = (randRow * nodeCount) - 1;


        graph[index] ^= 1;
        printf("original flip was %d\n", oldCount);
        newCount = CliqueCount(graph, nodeCount, oldCount);
        printf("flipping made count %d\n", newCount);
        if(newCount < oldCount) {
        	printf("good move here! at %d count\n", newCount);
        	oldCount = newCount;
			edgesPlaced++;
        } else {
        	graph[index] ^= 1;
        }
    }
}

void wipeLastColumn(int* graph, const int nodeCount) {
	int row;

	for(row = 0; row < nodeCount; row ++) {
		graph[row * nodeCount - 1] = 0;
	}
}

void writeToFile(FILE *fp, int *matrix, int counter_number) {
	int matrix_size;
	int i;
	int j;
	int value;
	int count = CliqueCount(matrix, counter_number, INT_MAX);
	
	fprintf(fp, "%d %d", counter_number, count);

	matrix_size = counter_number * counter_number;

	for(i = 0; i < counter_number; i++) {
		fwrite("\n", 1, 1, fp);
		for(j = 0; j < counter_number; j++) {
			value = get(i, j, matrix, counter_number);
			fprintf(fp, "%d ", value);
		}
	}
}

void copyMatrix(int* oldGraph, const int oldNodeCount, int* newGraph, const int newNodeCount){
    int row;
    int col;
    int i;
    //Set new graph to all zeros
    for(i = 0; i < newNodeCount*newNodeCount; i++){
        newGraph[i] = 0;
    }

    //Copy over the old matrix
    for(row = 0; row < oldNodeCount; row++){
        for(col = 0; col < oldNodeCount; col++){
            int index = row * oldNodeCount + col;
            int index2 = row * newNodeCount + col;
            newGraph[index2] = oldGraph[index];
        }
    }
}

int main(int argc, char **argv) {
	char buffer[1024];
	bzero(buffer, 1024);
	if(argc < 2) {
		printf("bad input\n");
		exit(1);
	}
	int LOWER = atoi(argv[1]);
	printf("Lower is %d\n", LOWER);
	sprintf(buffer, "%d.txt", LOWER);
	FILE *fp = fopen(buffer, "r");
	int *counter = (int*)malloc(sizeof(int) * LOWER*LOWER);
	bzero(counter, LOWER*LOWER);
	int *graph = (int*)malloc(sizeof(int) * (LOWER + 1)*(LOWER + 1));
	bzero(graph, (LOWER + 1)*(LOWER + 1));
	char c;
	int count;
	int bestCount = INT_MAX;
	int iterations;
	iterations = LOWER/5;

	int i = 0;

	if (fp) {
		while ((c = getc(fp)) != EOF) {
			counter[i++] = c - '0'; 
		}
		fclose(fp);
	} else {
		printf("bad file\n");
		exit(1);
	}
	// char ip[100];
	// hostname_to_ip("CoordinatorLoadBalancer-000988402024.lb.cloud.aristotle.ucsb.edu", ip);
	// FILE *fp1 = fopen("blah.txt", "w");
	// writeToFile(fp1, counter, LOWER);
	count = CliqueCount(counter, LOWER, INT_MAX);
	printf("Initial count is %d\n", count);
	copyMatrix(counter, LOWER, graph, LOWER + 1);
	printf("Performing %d iterations\n\n", iterations);
	while(iterations-- != 0) {
		printf("\nNext iteration:\n");
		permuteLastColumn(graph, LOWER + 1);
		count = CliqueCount(graph, LOWER + 1, bestCount);
		if(count < bestCount) {
			fp = fopen("299.txt", "w");
			writeToFile(fp, graph, LOWER + 1);
			fclose(fp);
			bestCount = count;
		}
		printf("Iteration %d: count is %d\n",iterations, count);
		wipeLastColumn(graph, LOWER + 1);
	}


	// while(iterations < 40) {
		randomizeLastColumn(graph, LOWER + 1, count);
	// }
	// iterateLastColumn(graph, LOWER + 1, count);
	return 0;
}
