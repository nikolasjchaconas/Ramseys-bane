#include "distributed_flip.h"
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>

int WRITE_TO_FILE;


int main() {

	//set whether you want counter examples written to disk
	//(condor does not allow this)
	// set to 1 or 0
	WRITE_TO_FILE = 0;

	// set thread type
	// can be either SINGLE_THREAD or MULTI_THREAD
	setThreads(SINGLE_THREAD);
	return 0;
}