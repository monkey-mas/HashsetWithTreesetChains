// Copyright (c) 2015 Masaru Nomura
// Released under the MIT license
// http://opensource.org/licenses/mit-license.php

#ifndef HASHSET_CHAIN_H
#define HASHSET_CHAIN_H

/*
 * DO NOT make NUM_THREADS bigger than HASHSET_TABLE_SIZE
 * It's not worth for you when intending to improve performance.
 * Also, they must be both positive numbers.
 */
#define HASHSET_TABLE_SIZE 31
#define NUM_THREADS 1

enum SET_OPERATION {
	ADD,
	REMOVE,
	FIND,
	RETAIN
};

struct hashset_chain {
	int size; /* total number of elements in set */
	struct tree_set *table[HASHSET_TABLE_SIZE];
};

struct task_data {
	enum SET_OPERATION operation;
	struct hashset_chain *setA;
	struct hashset_chain *setB;	// Not used for operations with ***array***
	int *  array_data;			// Not used for operations with ***set***
	int    array_size;			// Not used for operaitons with ***set***
};

struct thread_task {
	int	   from, to;	// Ranges of index to do set operation
	int	   success;		// 1 if operation succedded(e.g. success of add/remove or find element), otherwise 0
	struct task_data * data;
	/* Callback functions */
	int (*function_with_data)(struct tree_set*, int); 				// Not used for operation with ***set***
	int (*function_with_chain)(struct tree_set*, struct tree_set*); // Not used for operation with ***array***
};

struct hashset_chain *hashset_create_set();
void hashset_free_set(struct hashset_chain *set);
int  hashset_size(struct hashset_chain *set);
void hashset_update_size(struct hashset_chain *set);
int  hashset_add(struct hashset_chain *set, int data);
int  hashset_add_set(struct hashset_chain *setA, struct hashset_chain *setB);
int  hashset_add_array(struct hashset_chain *set, int *array_data, int  array_size);
int  hashset_remove(struct hashset_chain *set, int data);
int  hashset_remove_set(struct hashset_chain *setA, struct hashset_chain *setB);
int  hashset_remove_array(struct hashset_chain *set, int *array_data, int  array_size);
int  hashset_find(struct hashset_chain *set, int data);
int  hashset_find_set(struct hashset_chain *setA, struct hashset_chain *setB);
int  hashset_find_array(struct hashset_chain *set, int *array_data, int  array_size);
int  hashset_retain_set(struct hashset_chain *setA, struct hashset_chain *setB);
int  hashset_retain_array(struct hashset_chain *set, int *array_data, int  array_size);
int  hashset_union(struct hashset_chain *union_set, struct hashset_chain *setA, struct hashset_chain *setB);
int  hashset_intersection(struct hashset_chain *intersection_set, struct hashset_chain *setA, struct hashset_chain *setB);
int  hashset_difference(struct hashset_chain *difference_set, struct hashset_chain *setA, struct hashset_chain *setB);
int  hashset_symmetric_difference(struct hashset_chain *symmetric_difference_set, struct hashset_chain *setA, struct hashset_chain *setB);

#endif