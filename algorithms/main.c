#include "algorithms.h"
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>

int ALGORITHM_TYPE;
int WRITE_TO_FILE;
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
	ALGORITHM_TYPE = RANDOM;

	//set whether you want counter examples written to disk
	//(condor does not allow this)
	// set to 1 or 0
	WRITE_TO_FILE = 0;

	// set thread type
	// can be either SINGLE_THREAD or MULTI_THREAD
	setThreads(SINGLE_THREAD);
	return 0;
}