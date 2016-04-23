#include "cachelab.h"

int main(int argc, char **argv)
{
	options_t option = {0, 0, 0, 0, 0, 0};
	results_t result = {0, 0, 0};

	if (readParameters(argc, argv, &option) != RET_SUCC) {
		fprintf(stderr, "Error reading parameters from command line.\n");
		exit(1);
	}

	if (processTrace(&option, &result) != RET_SUCC) {
		exit(1);
	}
    printSummary(result.hit, result.miss, result.evict);
    return 0;
}
