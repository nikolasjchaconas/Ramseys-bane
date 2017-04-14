#include "thread.h"
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>

//4 threads is optimal
#define NUM_THREADS 4

int main() {
	int index;
	int bound;
	int clique_count;
	pthread_t *thread_array;
	pthread_mutex_t file_lock;
	int error;
	int thread_id;
	int i;
	int found;
	argStruct **arguments;

	found = 0;
	thread_array = (pthread_t *)malloc(sizeof(pthread_t) * NUM_THREADS);
	arguments = (argStruct **)malloc(sizeof(argStruct*) * NUM_THREADS);
	pthread_mutex_init(&file_lock, NULL);

	for(int thread_id = 0; thread_id < NUM_THREADS; thread_id++) {
		arguments[thread_id] = (argStruct *)malloc(sizeof(argStruct));
		arguments[thread_id]->thread_id = thread_id;
		arguments[thread_id]->found = &found;
		arguments[thread_id]->file_lock = &file_lock;
		error = pthread_create(&(thread_array[thread_id]), NULL, ThreadSolve, (void *)arguments[thread_id]);
		if(error) {
			fprintf(stderr, "error creating thread\n");
			exit(-1);
		}
	}

	//join threads
	for(i=0; i < NUM_THREADS; i++) {
		error = pthread_join(thread_array[i],NULL);
		if (error != 0) {
			fprintf(stderr, "error joining thread\n");
		}

	}

	return 0;
}