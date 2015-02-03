#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../hashset_chain.h"
#include "../treeset.h"

#ifndef TEST_SIZE
#define TEST_SIZE 1000000
#endif

int main(int argc, char const *argv[])
{
	struct hashset_chain *hset = hashset_create_set();
	int i;
	struct timespec begin, finish;
	double elapsed;

	/* Create test data */
	for (i = 0; i < TEST_SIZE; ++i)
	{
		hashset_add(hset, i);
	}

	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (i = 0; i < TEST_SIZE; ++i)
	{
		hashset_remove(hset, i);
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapsed = (finish.tv_sec - begin.tv_sec);
	elapsed += (finish.tv_nsec - begin.tv_nsec) / 1000000000.0;
	fprintf(stdout, "%f\n", elapsed);

	hashset_free_set(hset);
	return 0;
}