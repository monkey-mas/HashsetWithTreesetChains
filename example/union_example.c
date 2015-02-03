#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../hashset_chain.h"
#include "../treeset.h"

int main(int argc, char const *argv[])
{
	struct hashset_chain *hsetA, *hsetB, *set;
	int i, found;
	/* Create array data */
	int array_size = 10;
	int array[array_size];
	for (i = 0; i < array_size; ++i)
	{
		array[i] = i;
	}

	/* You can simply create a new set */
	hsetA = hashset_create_set();
	hsetB = hashset_create_set();
	set = hashset_create_set();
	if (hsetA == NULL || hsetB == NULL || set == NULL) {
		fprintf(stderr, "Oops... hashset was not creaded successfully...\n");
		return -1;
	}

	/* Create data */
	hashset_add_array(hsetA, array, 			 array_size/2); // hsetA has between 0 to 5	
	hashset_add_array(hsetB, array+array_size/2, array_size/2); // hsetA has between 5 to 10

	/* Union operation */
	fprintf(stdout, "********** UNION **********\n\n");
	fprintf(stdout, "Union of hsetA and hsetB\n");
	hashset_union(set, hsetA, hsetB);

	fprintf(stdout, "********** FIND **********\n\n");
	for (i = 0; i < array_size; ++i)
	{
		int found;
		found = hashset_find(set, i);
		if (found)
			fprintf(stdout, "%d : found \n", i);
		else
			fprintf(stdout, "%d : not found\n", i);
	}
	fprintf(stdout, "size : %d\n\n", set->size);

	hashset_free_set(hsetA);
	hashset_free_set(hsetB);
	hashset_free_set(set);
	return 0;
}