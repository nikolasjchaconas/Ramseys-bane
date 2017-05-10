#include "greedyGraphPermute.h"

const int getNextOneIndexInRow(int* graph, const int nodeCount, const int row, int* col){
	if(nodeCount > 0 && row >= 0 && row < nodeCount && *col >= 0 && *col < nodeCount){
		int index = (row*nodeCount)+(*col);
		while(graph[index] != 1){
			(*col)++;
			index = (row*nodeCount)+(*col);
		}
		return index;
	}
	else{
		printf("Error in getNextOneIndexInRow. The input was row: %d, col: %d, nodeCount: %d\n", row, *col, nodeCount);
		exit(0);
	}
}

const int getNextZeroIndexInRow(int* graph, const int nodeCount, const int row, int* col){

	if(nodeCount > 0 && row >= 0 && row < nodeCount && *col >= 0 && *col < nodeCount){
		int index = (row*nodeCount)+(*col);

		while(graph[index] != 0){
			(*col)++;
			index = (row*nodeCount)+(*col);
		}

		return index;
	}
	else{
		printf("Error in getNextZeroIndexInRow. The input was row: %d, col: %d, nodeCount: %d\n", row, *col, nodeCount);
		exit(0);
	}
}

const int getLastZeroIndexInRow(int* graph, const int nodeCount, const int row){
	if(nodeCount > 0 && row >= 0 && row < nodeCount){
		int index = (row*nodeCount)+(nodeCount - 1);
		while(graph[index]!= 0||index<row*nodeCount+row+1) index--;
		return index;
	}
	else{
		printf("Error in getLastZeroIndexInRow. The input was row: %d, nodeCount: %d\n", row, nodeCount);
		exit(0);
	}
}

const int getLastOneIndexInRow(int* graph, const int nodeCount, const int row){
	if(nodeCount > 0 && row >= 0 && row < nodeCount){
		int index = (row*nodeCount)+(nodeCount-1);
		while(graph[index]!= 1||index<row*nodeCount+row+1) index--;
		return index;
	}
	else{
		printf("Error in getLastOneIndexInRow. The input was row: %d, nodeCount: %d\n", row, nodeCount);
		exit(0);
	}
}

void switchEdges(int* graph, const int index1, const int index0){
	if(index1 >= 0 && index0 >= 0){
		graph[index1] = !graph[index1];
		graph[index0] = !graph[index0];
	}
	else{
		printf("Error in switchEdges. The input was index1: %d, index0: %d. Switch not performed. \n", index1, index0);
	}
}

const int greedyIndexPermute(int* graph, const int nodeCount, const int cliqueCount, const int index){

	if(graph[index] == 1 && nodeCount > 0 && cliqueCount >= 0 && index >= 0 && index < nodeCount*nodeCount){

		const int row = floor(index / nodeCount);
		const int lastZeroIndex = getLastZeroIndexInRow(graph, nodeCount, row);

		//Start with a valid value for the col
		int col = row + 1;
		int nextZeroIndex = (row * nodeCount) + col;

		while(nextZeroIndex < lastZeroIndex){
			nextZeroIndex = getNextZeroIndexInRow(graph, nodeCount, row, &col);
			switchEdges(graph, index, nextZeroIndex);
			const int newCliqueCount = CliqueCount(graph, nodeCount);
			printf("Checking out: %d\n",newCliqueCount);
			
			if(newCliqueCount < cliqueCount){
				printf("BestCount is: %d\n", newCliqueCount);
				return newCliqueCount;
			}

			switchEdges(graph, index, nextZeroIndex);

			col++;
			nextZeroIndex = (row*nodeCount)+col;
		}
	}
	else{
		printf("Error in greedyIndexPermute. Got the index: %d. It is pointing to the value: %d in the graph. The inputs were nodeCount: %d, cliqueCount: %d. PermuteEdge will not be performed.\n", index, graph[index], nodeCount, cliqueCount);
	}

	return cliqueCount;
}

const int greedyRowPermute(int* graph, const int nodeCount, const int cliqueCount, const int row){
	if(nodeCount > 0 && cliqueCount >= 0 && row >= 0 && row < nodeCount){
		//Pick the last 1 edge in the row
		const int lastOneIndex = getLastOneIndexInRow(graph, nodeCount, row);

		//Init basic vars used in the permute algorithm to valid values
		int col = row + 1;
		int nextOneIndex = (row * nodeCount) + col;

		while(nextOneIndex < lastOneIndex){
			nextOneIndex = getNextOneIndexInRow(graph, nodeCount, row, &col);
			const int newCliqueCount = greedyIndexPermute(graph, nodeCount, cliqueCount, nextOneIndex);
			if(newCliqueCount < cliqueCount) return newCliqueCount;
			col++;
			nextOneIndex = (row * nodeCount) + col;
		}
	}
	else{
		printf("Error in greedyRowPermute. Input was nodeCount: %d, cliqueCount: %d, row: %d. greedyRowPermute will not be performed.\n", nodeCount, cliqueCount, row);
	}

	//Did not find any better graphs
	return cliqueCount;
}

const int greedyGraphPermute(int* graph, const int nodeCount, const int cliqueCount){
	if(nodeCount > 0 && cliqueCount >= 0){
		//Iterate over all the rows
		int row;
		for(row = 0; row < nodeCount-2; row++){
			//Generate all permutations in row
			const int newCliqueCount = greedyRowPermute(graph, nodeCount, cliqueCount, row);
			if(newCliqueCount < cliqueCount) return newCliqueCount;
		}
	}
	else{
		printf("Error in greedyGraphPermute. Input was nodeCount: %d, cliqueCount: %d. greedyGraphPermute will not be performed\n", nodeCount, cliqueCount);
	}
	
	//Did not find any better graphs
	return cliqueCount;
}

void randomGraphPermute(int* graph, const int nodeCount, const int permuteFactor){
	if(nodeCount > 0 && permuteFactor > 0){
		for(int i = 0; i < permuteFactor; i++){
			int randRow = rand() % (nodeCount-2);
			int randCol = (rand() %  (nodeCount-randRow-1))+randRow+1;
			int index1 = (randRow*nodeCount)+randCol;
			int randCol2 = (rand() %  (nodeCount-randRow-1))+randRow+1;
			int index2 = (randRow*nodeCount)+randCol2;

			//Only switch two edges if they are oppsite in value, and in the same row to preserve the 50/50 distribution
			if((graph[index1] ^ graph[index2]) == 1){
				graph[index1] = !graph[index1];
				graph[index2] = !graph[index2];
			}
		}
	}
	else{
		printf("Error. Got invalid input in randomGraphPermute. The nodeCount was: %d and the permuteFactor was: %d.\n They both have to be positive\n", nodeCount, permuteFactor);
	}
}

void copyGraph(int* graph, int* copy, const int nodeCount){
	if(nodeCount > 0){
		const int matrixSize = nodeCount*nodeCount;
		int i;
		for(i = 0; i < matrixSize; i++){
			copy[i] = graph[i];
		}
	}
	else{
		printf("Error. Got invalid input in copyGraph. The nodeCount was: %d.\n It has to be positive\n", nodeCount);
	}
}

const int randomGraphExplore(int* graph, const int nodeCount, const int cliqueCount){
	if(nodeCount > 0 && cliqueCount >= 0){
		const int matrixSize = nodeCount*nodeCount;
		int graphCopy[matrixSize];
		copyGraph(graph, graphCopy, nodeCount);
		randomGraphPermute(graphCopy, nodeCount, 0.2*matrixSize);
		int newCliqueCount = CliqueCount(graphCopy, nodeCount);
		printf("currentCount is: %d\n",newCliqueCount);
		if(newCliqueCount < cliqueCount){
			copyGraph(graphCopy, graph, nodeCount);
			return newCliqueCount;
		}
	}
	else{
		printf("Error. Got invalid input in randomGraphExplore. The nodeCount was: %d. And the cliqueCount was %d.\n The nodeCount has to be positive and cliqueCount can't be below 0.\n", nodeCount, cliqueCount);
	}

	return cliqueCount;
}