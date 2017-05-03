#include "algorithms.h"
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>

int algorithm_type;

/*

basic options, check algorithms.h for most up to date.


//single vs multithread
SINGLE_THREAD
MULTI_THREAD

//algorithms
RANDOM
RANDOM_50_50

*/

int main() {
	// set algorithm type
	// check algorithms.h for more choices
	algorithm_type = RANDOM;

	// set thread type
	// can be either SINGLE_THREAD or MULTI_THREAD
	setThreads(MULTI_THREAD);
	return 0;
}