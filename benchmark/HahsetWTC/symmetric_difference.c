#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../hashset_chain.h"
#include "../treeset.h"

#ifndef TEST_SIZE
#define TEST_SIZE 2000000
#endif

int main(int argc, char const *argv[])
{
	struct hashset_chain *symmetric_difference = hashset_create_set();
	struct hashset_chain *hsetA = hashset_create_set();
	struct hashset_chain *hsetB = hashset_create_set();
	int i, *array;
	struct timespec begin, finish;
	double elapsed;

	/* Create test data */
	array = (int *)calloc(TEST_SIZE, sizeof(int));
	if (array == NULL) {
		perror("Failed to allocate memory to array");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < TEST_SIZE; ++i)
	{
		array[i] = i;
	}
	hashset_add_array(hsetA, array, TEST_SIZE/4 * 3);
	hashset_add_array(hsetB, array+TEST_SIZE/4, TEST_SIZE/4 * 3);

	clock_gettime(CLOCK_MONOTONIC, &begin);
	hashset_symmetric_difference(symmetric_difference, hsetA, hsetB);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapsed = (finish.tv_sec - begin.tv_sec);
	elapsed += (finish.tv_nsec - begin.tv_nsec) / 1000000000.0;
	fprintf(stdout, "%f\n", elapsed);

	hashset_free_set(symmetric_difference);
	hashset_free_set(hsetA);
	hashset_free_set(hsetB);
	free(array);
	return 0;
}