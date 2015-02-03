#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../hashset_chain.h"
#include "../treeset.h"

int main(int argc, char const *argv[])
{
	struct hashset_chain *hset, *hsetB;
	int i, found;
	/* Create array data */
	int array_size = 10;
	int array[array_size];
	for (i = 0; i < array_size; ++i)
	{
		array[i] = i;
	}

	/* You can simply create a new set */
	hset = hashset_create_set();
	hsetB = hashset_create_set();
	if (hset == NULL || hsetB == NULL) {
		fprintf(stderr, "Oops... hashset was not creaded successfully...\n");
		return -1;
	}

	/* Example of add/find */
	fprintf(stdout, "********** ADD **********\n\n");
	fprintf(stdout, "Add hset int from 0 to %d\n\n", array_size/2);

	for (i = 0; i < array_size/2; ++i)
	{
		hashset_add(hset, i);
	}
	fprintf(stdout, "Set size : %d\n\n", hset->size);

	fprintf(stdout, "********** FIND **********\n\n");
	for (i = 0; i < array_size; ++i)
	{
		found = hashset_find(hset, i);
		if (found)
			fprintf(stdout, "%d : found \n", i);
		else
			fprintf(stdout, "%d : not found\n", i);
	}
	fprintf(stdout, "\nSet size : %d\n\n", hset->size);


	/* Example of add_array/find */
	fprintf(stdout, "********** ADD_ARRAY **********\n\n");
	fprintf(stdout, "Add hsetB int array from %d to %d\n\n", array_size/2, array_size);

	hashset_add_array(hsetB, array+array_size/2, array_size/2);
	fprintf(stdout, "Set size : %d\n\n", hsetB->size);

	fprintf(stdout, "********** FIND **********\n\n");
	for (i = 0; i < array_size; ++i)
	{
		int found;
		found = hashset_find(hsetB, i);
		if (found)
			fprintf(stdout, "%d : found \n", i);
		else
			fprintf(stdout, "%d : not found\n", i);
	}
	fprintf(stdout, "\nSet size : %d\n\n", hsetB->size);


	/* Example of add_set/find */
	fprintf(stdout, "********** ADD SET **********\n\n");
	fprintf(stdout, "Add hsetB to hset\n\n");
	hashset_add_set(hset, hsetB);

	fprintf(stdout, "********** FIND **********\n\n");
	for (i = 0; i < array_size; ++i)
	{
		int found;
		found = hashset_find(hset, i);
		if (found)
			fprintf(stdout, "%d : found \n", i);
		else
			fprintf(stdout, "%d : not found\n", i);
	}
	fprintf(stdout, "size : %d\n\n", hset->size);

	hashset_free_set(hset);
	hashset_free_set(hsetB);
	return 0;
}