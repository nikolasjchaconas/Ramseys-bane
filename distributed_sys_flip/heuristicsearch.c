#include "heuristicsearch.h"
#include <pthread.h>

int removeMeLaterBro;

boolean addSet(set_t s, graph_t *g, clique_options *opts){
	opts->clique_list[opts->clique_list_length] = set_duplicate(s);
	opts->clique_list_length++;
	if(opts->clique_list_length >= removeMeLaterBro) return TRUE;
	return TRUE;
}

void getMaxCliqueParticipatingNode(set_t* cliques, const int nrCliques, const int nodeCount, set_t *worstNodeArray){
	int i;
	set_t unionSet = set_new(nodeCount);
	for(i = 0; i < nrCliques; i++){
		set_t tmp = set_duplicate(unionSet);
		set_union(unionSet, tmp, cliques[i]);
		set_free(tmp);
	}

	int nodeInCliqueCount = 0;
	// printf("this is the union set bro\n");
	// set_print(unionSet);

	int cliqueNode = -1;
	while((cliqueNode = set_return_next(unionSet, cliqueNode))>=0){
		nodeInCliqueCount = 0;
		for(i = 0; i < nrCliques; i++){
			set_t s = cliques[i];
			if(SET_CONTAINS(s, cliqueNode)) nodeInCliqueCount++;
		}

		if(worstNodeArray[nodeInCliqueCount] == NULL){
			worstNodeArray[nodeInCliqueCount] = set_new(nodeCount);
		}
		SET_ADD_ELEMENT(worstNodeArray[nodeInCliqueCount], cliqueNode);
	}
	set_free(unionSet);
}

const int getCliquesWithNode(const int node, set_t* cliques, const int cliqueNr, set_t* cliquesWithNode){
	int i;
	int j = 0;
	for(i = 0; i < cliqueNr; i++){
		set_t s = cliques[i];
		if(SET_CONTAINS(s, node)) cliquesWithNode[j++] = set_duplicate(s);
	}
	return j;
}

void initGraphs(int* g, const int nodeCount, graph_t* typeOneGraph, graph_t* typeZeroGraph){
	int i;
	for(i = 0; i < nodeCount*nodeCount; i++){
		int col = i%nodeCount;
		int row = (int) floor(i/nodeCount);
		if(g[i] == 1 && row < col ){
			GRAPH_ADD_EDGE(typeOneGraph, row, col);
		}
		else if(g[i] == 0 && row < col){
			GRAPH_ADD_EDGE(typeZeroGraph, row, col);
		}
	}
}

void constructGraph(graph_t* typeOne, int* g, const int nodeCount){
  const int graphSize = nodeCount*nodeCount;
  int row;
  int i;
  for(i = 0; i < graphSize; i++) g[i] = 0;

  // printf("HEI\n");
  for(row = 0; row < nodeCount; row++){
    set_t s = typeOne->edges[row];
    // set_print(s);
    int col = -1;
    while((col = set_return_next(s, col))>=0){
      if(row < col){
        int index = (row * nodeCount) + col;
        g[index] = 1;
      }
    }
  }

}

void sendGraphToCoordinator(graph_t *typeOneGraph, const int nodeCount, client_struct* client_info, int index, int cliqueCount) {
	int *g = (int *)malloc(sizeof(int)*nodeCount*nodeCount);

	constructGraph(typeOneGraph, g, nodeCount);
	
	sendCounterExampleToCoordinator(nodeCount, cliqueCount, 0, g, client_info);
}

int replaceMe(int* g, int nodeCount, int cliqueCount, client_struct* client_info) {
	int j;
	int flipThreshold = 20;
	set_t typeOneCliqueSets[cliqueCount*100];
	clique_options typeOneOptions = {
		reorder_by_default, NULL, clique_print_time, NULL, addSet, NULL, typeOneCliqueSets, 0
	};
	set_t typeZeroCliqueSets[cliqueCount*100];
	clique_options typeZeroOptions = {
		reorder_by_default, NULL, clique_print_time, NULL, addSet, NULL, typeZeroCliqueSets, 0
	};
	set_t worstTypeOneSet;
	set_t worstTypeZeroSet;
	set_t secondWorstTypeOneSet;
	set_t secondWorstTypeZeroSet;
	graph_t* typeOneGraph = graph_new(nodeCount);
	graph_t* typeZeroGraph = graph_new(nodeCount);

	initGraphs(g, nodeCount, typeOneGraph, typeZeroGraph);

	int typeZeroCliqueCount = clique_find_all(typeZeroGraph, 10, 10, FALSE, &typeZeroOptions);
	int typeOneCliqueCount = clique_find_all(typeOneGraph, 10, 10, FALSE, &typeOneOptions);
	int worstTypeOneNodeSetCount;
	int worstTypeZeroNodeSetCount;

	// Add coordinator talk
	while(typeZeroCliqueCount + typeOneCliqueCount > 0 ) {
		const int worstTypeArraySize = typeZeroCliqueCount + typeOneCliqueCount;
		set_t worstTypeOneNodeArray[worstTypeArraySize];
		for(j = 0; j < worstTypeArraySize; j++){
			worstTypeOneNodeArray[j] = NULL;
		}
		set_t worstTypeZeroNodeArray[worstTypeArraySize];
		for(j = 0; j < worstTypeArraySize; j++){
			worstTypeZeroNodeArray[j] = NULL;
		}

		set_t secondWorstTypeOneNodeArray[worstTypeArraySize];
		for(j = 0; j < worstTypeArraySize; j++){
			secondWorstTypeOneNodeArray[j] = NULL;
		}
		set_t secondWorstTypeZeroNodeArray[worstTypeArraySize];
		for(j = 0; j < worstTypeArraySize; j++){
			secondWorstTypeZeroNodeArray[j] = NULL;
		}

		// type zero stuff
		printf("\n\nBeginning Zero Flips\n");
		getMaxCliqueParticipatingNode(typeZeroCliqueSets, typeZeroCliqueCount, nodeCount, worstTypeZeroNodeArray);
		for(j = typeZeroCliqueCount - 1; j >= 0; j--) {
			int failures = 0;
			worstTypeZeroSet = worstTypeZeroNodeArray[j];
			if(worstTypeZeroSet != NULL) {
				int foundBest = 0;
				int worstNode = -1;
				int secondWorstNode = -1;

				worstTypeZeroNodeSetCount = worstTypeArraySize;
				set_t zeroCliquesWithWorstNode[worstTypeZeroNodeSetCount];

				int m;
				for(m = 0; m < worstTypeZeroNodeSetCount; m++) {
					zeroCliquesWithWorstNode[m] = NULL;
				}
				// printf("Set to Iterate with Occurance of %d:\n", j);
				// set_print(worstTypeZeroSet);
				// printf("----------\n");
				//find worst adjacent node to z
				while((worstNode = set_return_next(worstTypeZeroSet, worstNode)) >= 0) {
					// printf("Here is the worst node we are on: %d, and below is worst set\n", worstNode);
					// set_print(worstTypeZeroSet); 
					int k;
					const int cliquesWithNodeCount = getCliquesWithNode(worstNode, typeZeroCliqueSets, typeZeroCliqueCount, zeroCliquesWithWorstNode);

					for(m = 0; m < worstTypeArraySize; m++){
						if(secondWorstTypeZeroNodeArray[m] != NULL) {
							set_free(secondWorstTypeZeroNodeArray[m]);
							secondWorstTypeZeroNodeArray[m] = NULL;
						}
					}

					getMaxCliqueParticipatingNode(zeroCliquesWithWorstNode, cliquesWithNodeCount, nodeCount, secondWorstTypeZeroNodeArray);
					
					for(k = worstTypeArraySize - 1; k >= 0; k--){
						secondWorstTypeZeroSet = secondWorstTypeZeroNodeArray[k];
						if(secondWorstTypeZeroSet != NULL && set_size(secondWorstTypeZeroSet) != 1) {
							SET_DEL_ELEMENT(secondWorstTypeZeroSet, worstNode);
							break;
						}
					}

					// set_print(secondWorstTypeZeroSet);
					// set_print(secondWorstTypeZeroSet);
					// for(k = 0; k < cliquesWithNodeCount; k ++) {
					// 	set_print(cliquesWithWorstNode[k]);
					// }
					secondWorstNode = -1;
					while((secondWorstNode = set_return_next(secondWorstTypeZeroSet, secondWorstNode)) >= 0) {
						printf("\n\n----------------------Iteration---------------------------\n");
						printf("Comparing worst: %d, and secondWorse: %d\n", worstNode, secondWorstNode);
						printf("Worst: ");
						set_print(worstTypeZeroSet);
						printf("Second Worst:");
						set_print(secondWorstTypeZeroSet);
						
						//relationship is between worstNode and secondWorstNode
						GRAPH_DEL_EDGE(typeZeroGraph, worstNode, secondWorstNode);
						GRAPH_ADD_EDGE(typeOneGraph, worstNode, secondWorstNode);

						clique_options tempOneOptions = {
							reorder_by_default, NULL, clique_print_time, NULL, NULL, NULL, NULL, 0
						};
						clique_options tempZeroOptions = {
							reorder_by_default, NULL, clique_print_time, NULL, NULL, NULL, NULL, 0
						};

						int tempOneCount = clique_find_all(typeOneGraph, 10, 10, FALSE, &tempOneOptions);
						int tempZeroCount = clique_find_all(typeZeroGraph, 10, 10, FALSE, &tempZeroOptions);

						if(tempOneCount + tempZeroCount < typeOneCliqueCount + typeZeroCliqueCount) {
							printf("\nZero Sets: Success!\nTotal %d => %d\nZero Count %d => %d\nOne Count %d => %d\n----------------------------------------------------------\n\n", 
								typeZeroCliqueCount + typeOneCliqueCount, tempOneCount + tempZeroCount, 
								typeZeroCliqueCount, tempZeroCount, typeOneCliqueCount, tempOneCount);
							typeOneCliqueCount = tempOneCount;
							typeZeroCliqueCount = tempZeroCount;
							int lm;
							for(lm = 0; lm < 5; lm++) {
								sendGraphToCoordinator(typeOneGraph, nodeCount, client_info, 0, typeOneCliqueCount + typeZeroCliqueCount);
							}
							if((client_info->coordinator_return->counter_number > nodeCount) || (client_info->coordinator_return->clique_count < (typeZeroCliqueCount + typeOneCliqueCount))) {
								//bail brotha
								constructGraph(typeOneGraph, g, nodeCount);
								return typeOneCliqueCount + typeZeroCliqueCount;
							}
							foundBest = 1;
							break;
						} else {
							printf("\nZero Sets: Failure.\nTotal %d => %d\nZero Count %d => %d\nOne Count %d => %d\n----------------------------------------------------------\n\n", 
								typeZeroCliqueCount + typeOneCliqueCount, tempOneCount + tempZeroCount, 
								typeZeroCliqueCount, tempZeroCount, typeOneCliqueCount, tempOneCount);
							GRAPH_ADD_EDGE(typeZeroGraph, worstNode, secondWorstNode);
							GRAPH_DEL_EDGE(typeOneGraph, worstNode, secondWorstNode);

							if(++failures % flipThreshold == 0) {
								printf("\nT%d: %d flips have been made, polling coordinator!\n", client_info->id, flipThreshold);
								sendGraphToCoordinator(typeOneGraph, nodeCount, client_info, 0, typeOneCliqueCount + typeZeroCliqueCount);
								if((client_info->coordinator_return->counter_number > nodeCount) || (client_info->coordinator_return->clique_count < (typeZeroCliqueCount + typeOneCliqueCount))) {
									//bail brotha
									constructGraph(typeOneGraph, g, nodeCount);
									return typeOneCliqueCount + typeZeroCliqueCount;
								}
							}
						}
					}
					if(foundBest) {
						break;
					}
				}
				printf("resetting here1\n");
				for(m = 0; m < worstTypeZeroNodeSetCount; m++) {
					if(zeroCliquesWithWorstNode[m] != NULL) {
						set_free(zeroCliquesWithWorstNode[m]);
						zeroCliquesWithWorstNode[m] = NULL;
					}
				}
				if(foundBest) {
					break;
				}
			}
		}
		//free resources
		int it;
		printf("freeing here\n");
		for (it = 0; it < cliqueCount*10; ++it)
		{
			typeOneCliqueSets[it] = NULL;
			typeZeroCliqueSets[it] = NULL;
		}

		for(j = 0; j < worstTypeArraySize; j++){
			if(worstTypeZeroNodeArray[j] != NULL) {
				set_free(worstTypeZeroNodeArray[j]);
				worstTypeZeroNodeArray[j] = NULL;
			}
		}
		for(j = 0; j < worstTypeArraySize; j++){
			if(secondWorstTypeZeroNodeArray[j] != NULL) {
				set_free(secondWorstTypeZeroNodeArray[j]);
				secondWorstTypeZeroNodeArray[j] = NULL;
			}
		}

		typeOneOptions.clique_list_length = 0;
		typeZeroOptions.clique_list_length = 0;

		clique_find_all(typeOneGraph, 10, 10, FALSE, &typeOneOptions);
		clique_find_all(typeZeroGraph, 10, 10, FALSE, &typeZeroOptions);
		//end of type zero stuff

		// type one stuff
		printf("\n\nBeginning One Flips\n");
		getMaxCliqueParticipatingNode(typeOneCliqueSets, typeOneCliqueCount, nodeCount, worstTypeOneNodeArray);
		for(j = typeOneCliqueCount - 1; j >= 0; j--) {
			int failures = 0;
			worstTypeOneSet = worstTypeOneNodeArray[j];
			if(worstTypeOneSet != NULL) {
				int foundBest = 0;
				int worstNode = -1;
				int secondWorstNode = -1;

				worstTypeOneNodeSetCount = worstTypeArraySize;
				set_t OneCliquesWithWorstNode[worstTypeOneNodeSetCount];

				int m;
				for(m = 0; m < worstTypeOneNodeSetCount; m++) {
					OneCliquesWithWorstNode[m] = NULL;
				}
				// printf("Set to Iterate with Occurance of %d:\n", j);
				// set_print(worstTypeOneSet);
				// printf("----------\n");
				//find worst adjacent node to z
				while((worstNode = set_return_next(worstTypeOneSet, worstNode)) >= 0) {
					// printf("Here is the worst node we are on: %d, and below is worst set\n", worstNode);
					// set_print(worstTypeOneSet); 
					int k;
					const int cliquesWithNodeCount = getCliquesWithNode(worstNode, typeOneCliqueSets, typeOneCliqueCount, OneCliquesWithWorstNode);

					for(m = 0; m < worstTypeArraySize; m++){
						if(secondWorstTypeOneNodeArray[m] != NULL) {
							set_free(secondWorstTypeOneNodeArray[m]);
							secondWorstTypeOneNodeArray[m] = NULL;
						}
					}

					getMaxCliqueParticipatingNode(OneCliquesWithWorstNode, cliquesWithNodeCount, nodeCount, secondWorstTypeOneNodeArray);
					
					for(k = worstTypeArraySize - 1; k >= 0; k--){
						secondWorstTypeOneSet = secondWorstTypeOneNodeArray[k];
						if(secondWorstTypeOneSet != NULL && set_size(secondWorstTypeOneSet) != 1) {
							SET_DEL_ELEMENT(secondWorstTypeOneSet, worstNode);
							break;
						}
					}

					// set_print(secondWorstTypeOneSet);
					// set_print(secondWorstTypeOneSet);
					// for(k = 0; k < cliquesWithNodeCount; k ++) {
					// 	set_print(cliquesWithWorstNode[k]);
					// }
					secondWorstNode = -1;
					while((secondWorstNode = set_return_next(secondWorstTypeOneSet, secondWorstNode)) >= 0) {
						printf("\n\n----------------------Iteration---------------------------\n");
						printf("Comparing worst: %d, and secondWorse: %d\n", worstNode, secondWorstNode);
						printf("Worst: ");
						set_print(worstTypeOneSet);
						printf("Second Worst:");
						
						set_print(secondWorstTypeOneSet);
						//relationship is between worstNode and secondWorstNode
						GRAPH_DEL_EDGE(typeOneGraph, worstNode, secondWorstNode);
						GRAPH_ADD_EDGE(typeZeroGraph, worstNode, secondWorstNode);

						clique_options tempZeroOptions = {
							reorder_by_default, NULL, clique_print_time, NULL, NULL, NULL, NULL, 0
						};
						clique_options tempOneOptions = {
							reorder_by_default, NULL, clique_print_time, NULL, NULL, NULL, NULL, 0
						};

						int tempZeroCount = clique_find_all(typeZeroGraph, 10, 10, FALSE, &tempZeroOptions);
						int tempOneCount = clique_find_all(typeOneGraph, 10, 10, FALSE, &tempOneOptions);

						if(tempZeroCount + tempOneCount < typeZeroCliqueCount + typeOneCliqueCount) {
							printf("\nOne Sets: Success!\nTotal %d => %d\nOne Count %d => %d\nZero Count %d => %d\n----------------------------------------------------------\n\n", 
								typeOneCliqueCount + typeZeroCliqueCount, tempZeroCount + tempOneCount, 
								typeOneCliqueCount, tempOneCount, typeZeroCliqueCount, tempZeroCount);
							typeZeroCliqueCount = tempZeroCount;
							typeOneCliqueCount = tempOneCount;
							int lm;
							for(lm = 0; lm < 5; lm++) {
								sendGraphToCoordinator(typeOneGraph, nodeCount, client_info, 0, typeZeroCliqueCount + typeOneCliqueCount);
							}
							if((client_info->coordinator_return->counter_number > nodeCount) || (client_info->coordinator_return->clique_count < (typeOneCliqueCount + typeZeroCliqueCount))) {
								//bail brotha
								printf("bailing!\n");
								constructGraph(typeOneGraph, g, nodeCount);
								return typeZeroCliqueCount + typeOneCliqueCount;
							}
							foundBest = 1;
							break;
						} else {
							printf("\nOne Sets: Failure.\nTotal %d => %d\nOne Count %d => %d\nZero Count %d => %d\n----------------------------------------------------------\n\n", 
								typeOneCliqueCount + typeZeroCliqueCount, tempZeroCount + tempOneCount, 
								typeOneCliqueCount, tempOneCount, typeZeroCliqueCount, tempZeroCount);
							GRAPH_ADD_EDGE(typeOneGraph, worstNode, secondWorstNode);
							GRAPH_DEL_EDGE(typeZeroGraph, worstNode, secondWorstNode);

							if(++failures % flipThreshold == 0) {
								printf("\nT%d: %d flips have been made, polling coordinator!\n", client_info->id, flipThreshold);
								sendGraphToCoordinator(typeOneGraph, nodeCount, client_info, 0, typeZeroCliqueCount + typeOneCliqueCount);
								if((client_info->coordinator_return->counter_number > nodeCount) || (client_info->coordinator_return->clique_count < (typeOneCliqueCount + typeZeroCliqueCount))) {
									//bail brotha
									printf("bailing!\n");
									constructGraph(typeOneGraph, g, nodeCount);
									return typeZeroCliqueCount + typeOneCliqueCount;
								}
							}
						}
					}
					if(foundBest) {
						break;
					}
				}
				printf("resetting here1\n");
				for(m = 0; m < worstTypeOneNodeSetCount; m++) {
					if(OneCliquesWithWorstNode[m] != NULL) {
						set_free(OneCliquesWithWorstNode[m]);
						OneCliquesWithWorstNode[m] = NULL;
					}
				}
				if(foundBest) {
					break;
				}
			}
		}
		//free resources
		printf("freeing here\n");
		for (it = 0; it < cliqueCount*10; ++it)
		{
			typeOneCliqueSets[it] = NULL;
			typeOneCliqueSets[it] = NULL;
		}

		for(j = 0; j < worstTypeArraySize; j++){
			if(worstTypeOneNodeArray[j] != NULL) {
				set_free(worstTypeOneNodeArray[j]);
				worstTypeOneNodeArray[j] = NULL;
			}
		}
		for(j = 0; j < worstTypeArraySize; j++){
			if(secondWorstTypeOneNodeArray[j] != NULL) {
				set_free(secondWorstTypeOneNodeArray[j]);
				secondWorstTypeOneNodeArray[j] = NULL;
			}
		}

		typeOneOptions.clique_list_length = 0;
		typeOneOptions.clique_list_length = 0;

		clique_find_all(typeOneGraph, 10, 10, FALSE, &typeOneOptions);
		clique_find_all(typeOneGraph, 10, 10, FALSE, &typeOneOptions);
		//end of type one stuff
	}
	//we found zero, send it baby
	constructGraph(typeOneGraph, g, nodeCount);
	return typeOneCliqueCount + typeZeroCliqueCount;
}