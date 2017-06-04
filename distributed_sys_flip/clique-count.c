#include "clique-count.h"

__thread int currentCliqueCount = 0;

boolean validateCount(set_t s, graph_t *g, clique_options *opts){
  opts->clique_list_length++;
  //printf("COUNT IS: %i\n", opts->clique_list_length);
  if(opts->clique_list_length > currentCliqueCount){
    return FALSE;
  }
  return TRUE;
}

int CliqueCount(int* g, int gsize, int currentClique){
  currentCliqueCount = currentClique;
  int i, verifySize;
  graph_t* typeOne = graph_new(gsize);
  graph_t* typeZero = graph_new(gsize);
  for(i = 0; i < gsize*gsize; i++){
    int col = i%gsize;
    int row = (int) floor(i/gsize);
    if(g[i] == 1 || g[i] == 0) verifySize++;
    if(g[i] == 1){
      GRAPH_ADD_EDGE(typeOne, row, col);
    }
    else if(g[i] == 0 && row < col){
      GRAPH_ADD_EDGE(typeZero, row, col);
    }
  }

  // The size of the graph is wrong
  if(verifySize < gsize*gsize) return currentClique;

  clique_options ramseyCounterOpts = {
  	reorder_by_default, NULL, clique_print_time, NULL, validateCount, NULL, NULL, 0
  };

  int typeOneCount = clique_find_all(typeOne, 10, 10, FALSE, &ramseyCounterOpts);
  graph_free(typeOne);
  if(typeOneCount > currentClique){
    return typeOneCount;
  }
  int typeZeroCount = clique_find_all(typeZero, 10, 10, FALSE, &ramseyCounterOpts);
  graph_free(typeZero);
  return ramseyCounterOpts.clique_list_length;
}
