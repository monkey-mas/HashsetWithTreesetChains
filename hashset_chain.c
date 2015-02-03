// Copyright (c) 2015 Masaru Nomura
// Released under the MIT license
// http://opensource.org/licenses/mit-license.php

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "hashset_chain.h"
#include "treeset.h"

/* only used for operations with ARRAY */
static pthread_once_t atmostonece_for_table_lock_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t table_locks[HASHSET_TABLE_SIZE];

static int  hashset_hash_code(int data);
static int  hashset_operation_template_with_data(struct hashset_chain *set, int data, int (*treeset_function)(struct tree_set*, int));
static int  hashset_operation_template_with_set(enum SET_OPERATION operation, struct hashset_chain *setA, struct hashset_chain *setB);
static int  hashset_operation_template_with_array(enum SET_OPERATION operation, struct hashset_chain *set, int *array_data, int  array_size);
static void hashset_table_lock_mutex_initialization(void);
static int  hashset_set_operation(enum SET_OPERATION operation, struct hashset_chain *setA, struct hashset_chain *setB, int *array_data,int  array_size);
static int  hashset_create_thread(pthread_t *tid, struct task_data *data, struct thread_task *tasks, int num_threads);
static int  hashset_compute_proper_number_of_threads(struct hashset_chain *setB, int array_size);
static void hashset_setup_task_data(struct task_data *data, enum SET_OPERATION operation, struct hashset_chain *setA, struct hashset_chain *setB, int *array_data, int  array_size);
static void hashset_setup_thread_task(struct task_data *data, struct thread_task *task, int from, int to);
static void *hashset_thread_operation(void *arg);
static void hashset_operate_with_all_elements_of_set(struct thread_task *task);
static void hashset_operate_with_all_elements_of_array(struct thread_task *task);

/*
 * Create hashset with initialization of table as well
 * @return pointer to a new set
 */
struct hashset_chain*
hashset_create_set()
{
	int i, j;
	struct hashset_chain *set;
	struct tree_set *chain;

	set = (struct hashset_chain *)malloc(sizeof(struct hashset_chain));
	if (set == NULL)
		return NULL;

	/* Initialize set */
	set->size = 0;
	for (i=0; i<HASHSET_TABLE_SIZE; i++) {
		chain = treeset_create_set();
		if (chain == NULL)
			goto free_set;

		set->table[i] = chain;
	}
	goto succeess;


free_set:
	for (j = 0; j < i; ++j)
	{
		chain = set->table[j];
		treeset_free_set(chain);
	}
	free(set);
	return NULL;

succeess:
	return set;
}

/*
 * Free set and chains
 */
void
hashset_free_set(struct hashset_chain *set)
{
	int i;
	struct tree_set *chain;

	if (set == NULL)
		return;

	for (i=0; i<HASHSET_TABLE_SIZE; i++) {
		chain = set->table[i];
		treeset_free_set(chain);
	}

	free(set);
}

/*
 * FIXME(mas): too simple to distribute data equally to table...
 */
static int
hashset_hash_code(int data)
{
	return data % HASHSET_TABLE_SIZE;
}

/*
 * @return size of total number of elements in set
 *
 * Time complexity O(1)
 * as we just search hash table and table size is constant HASHSET_TABLE_SIZE
 */
int
hashset_size(struct hashset_chain *set)
{
	int i, total;
	struct tree_set *treeset;

	if  (set == NULL) return 0;

	total = 0;
	for (i = 0; i < HASHSET_TABLE_SIZE; i++)
	{
		treeset = (set->table)[i];
		total  += treeset->size;
	}
	return total;
}

/*
 * Update the size of set
 */
void
hashset_update_size(struct hashset_chain *set) {
	set->size = hashset_size(set);
}

/*
 * Add data into set
 *
 * @return 1 if the data is added into set, 0 otherwise.
 */
int
hashset_add(struct hashset_chain *set,
			int data)
{
	int modified;
	int (*add_func)(struct tree_set *, int);

	add_func = &treeset_add;
	modified = hashset_operation_template_with_data(set, data, add_func);

	return modified;
}

/*
 * Add setB into setA
 *
 * @return 1 if some elements in setB are added into setA, 0 otherwise.
 */
int
hashset_add_set(struct hashset_chain *setA,
				struct hashset_chain *setB)
{
	enum SET_OPERATION operation;
	int  modified;

	operation = ADD;
	modified  = hashset_operation_template_with_set(operation, setA, setB);

	return modified;
}

/*
 * Add array into set
 *
 * @return 1 if some elements in the array are added into set, 0 otherwise.
 */
int
hashset_add_array(struct hashset_chain *set,
				  int *array_data,
				  int  array_size)
{
	enum SET_OPERATION operation;
	int modified;

	operation = ADD;
	modified  = hashset_operation_template_with_array(operation, set, array_data, array_size);

	return modified;
}

/*
 * Remove data from set
 *
 * @return 1 if the data is remove from the set, 0 otherwise.
 */
int
hashset_remove(struct hashset_chain *set,
			   int data)
{
	int modified;
	int (*rm_func)(struct tree_set *, int);

	rm_func  = &treeset_remove;
	modified = hashset_operation_template_with_data(set, data, rm_func);

	return modified;
}

/*
 * Remove setB from setA
 *
 * @return 1 if some elements in setB are removed from setA, 0 otherwise.
 */
int
hashset_remove_set(struct hashset_chain *setA,
				   struct hashset_chain *setB)
{
	enum SET_OPERATION operation;
	int  modified;

	operation = REMOVE;
	modified  = hashset_operation_template_with_set(operation, setA, setB);

	return modified;
}

/*
 * Remove array from set
 *
 * @return 1 if some elements in the array are removed from set, 0 otherwise.
 */
int
hashset_remove_array(struct hashset_chain *set,
					 int *array_data,
					 int  array_size)
{
	enum SET_OPERATION operation;
	int modified;

	operation = REMOVE;
	modified  = hashset_operation_template_with_array(operation, set, array_data, array_size);

	return modified;
}

/*
 * Check if data is in set
 *
 * @return 1 if true 0 otherwise
 */
int
hashset_find(struct hashset_chain *set,
			 int data)
{
	int found;
	int (*find_func)(struct tree_set *, int);

	find_func = &treeset_find;
	found	  = hashset_operation_template_with_data(set, data, find_func);

	return found;
}

/*
 * Check if all elements in setB are in setA
 *
 * @return 1 if true 0 otherwise
 */
int
hashset_find_set(struct hashset_chain *setA,
				 struct hashset_chain *setB)
{
	enum SET_OPERATION operation;
	int  found_all;

	operation = FIND;
	found_all = hashset_operation_template_with_set(operation, setA, setB);

	return found_all;
}

/*
 * Check if all elements in array are in set
 * @return 1 if true 0 otherwise
 */
int
hashset_find_array(struct hashset_chain *set,
				   int *array_data,
				   int  array_size)
{
	enum SET_OPERATION operation;
	int found_all;

	operation = FIND;
	found_all = hashset_operation_template_with_array(operation, set, array_data, array_size);

	return found_all;
}

/*
 * Intersect setA with setB
 * After the operation, setA contains elements that are only found in setB
 *
 * @return 0 if operation succeeded, 0 otherwise
 */
int
hashset_retain_set(struct hashset_chain *setA,
				   struct hashset_chain *setB)
{
	enum SET_OPERATION operation;
	int  success;

	operation = RETAIN;
	success	  = hashset_operation_template_with_set(operation, setA, setB);

	return success;
}

/*
 * Intersect set with array elements
 * After the operation, set contains elements that are only found in array
 *
 * By its nature of retain operation, we first create setB using array.
 * With this, we can maintain the simplicity of our APIs and make use of threads.
 *
 * @return 1 if true 0 otherwise
 */
int
hashset_retain_array(struct hashset_chain *set,
					 int *array_data,
					 int  array_size)
{
	
	enum SET_OPERATION operation, operation2;
	struct hashset_chain *setB;
	int success;

	/* Create setB using array_data */
	setB = hashset_create_set();
	operation = ADD;
	success	  = hashset_operation_template_with_array(operation, setB, array_data, array_size);
	if (!success)
		goto end;

	operation2 = RETAIN;
	success	   = hashset_operation_template_with_set(operation2, set, setB);

end:
	hashset_free_set(setB);
	return success;
}

/*
 * Union operation of setA and setB. The input union_set is the set that stores
 * elements of setA and setB, thus it SHOULD be an empty set provided by caller function.
 *
 * @return 1 if some elements are added to union_set, 0 otherwise.
 */
int
hashset_union(struct hashset_chain *union_set,
			  struct hashset_chain *setA,
			  struct hashset_chain *setB)
{
	int modified;

	if (union_set == NULL ||
		setA == NULL	  ||
		setB == NULL)
		return 0;

	modified = 0;
	modified |= hashset_add_set(union_set, setA);
	modified |= hashset_add_set(union_set, setB);

	return modified;
}

/*
 * Intersection operation of setA and setB. The input intersection_set is the set that stores
 * intersection elements of setA and setB, thus it SHOULD be an empty set provided by caller function.
 *
 * @return 1 if some elements are added to intersection_set, 0 otherwise.
 * TODO(mas): Need to improve the algorithm in treeset.c as well
 */
int
hashset_intersection(struct hashset_chain *intersection_set,
					 struct hashset_chain *setA,
					 struct hashset_chain *setB)
{
	int modified;

	if (intersection_set == NULL ||
		setA == NULL ||
		setB == NULL)
		return 0;

	modified = 0;
	modified |= hashset_add_set(intersection_set, setA);
	modified |= hashset_retain_set(intersection_set, setB);

	return modified;
}

/*
 * Difference operation of setA and setB. The input difference_set is the set that stores
 * difference elements of setA and setB, thus it SHOULD be an empty set provided by caller function.
 *
 * @return 1 if some elements are added to difference_set, 0 otherwise.
 * TODO(mas): Need to improve the algorithm in treeset.c as well
 */
int
hashset_difference(struct hashset_chain *difference_set,
				   struct hashset_chain *setA,
				   struct hashset_chain *setB)
{
	int modified;

	if (difference_set == NULL ||
		setA == NULL ||
		setB == NULL)
		return 0;

	modified = 0;
	modified |= hashset_add_set(difference_set, setA);
	modified |= hashset_remove_set(difference_set, setB);

	return modified;
}

/*
 * Intersection operation of setA and setB. The input symmetric_difference_set is the set that stores
 * intersection elements of setA and setB, thus it SHOULD be an empty set provided by caller function.
 *
 * @return 1 if some elements are added to symmetric_difference_set, 0 otherwise.
 * TODO(mas): Need to improve the algorithm in treeset.c as well
 */
int
hashset_symmetric_difference(struct hashset_chain *symmetric_difference_set,
							 struct hashset_chain *setA,
							 struct hashset_chain *setB)
{
	int modified;
	struct hashset_chain *right_half_set;

	if (symmetric_difference_set == NULL ||
		setA == NULL ||
		setB == NULL)
		return 0;

	right_half_set = hashset_create_set();
	if (right_half_set == NULL)
		exit(EXIT_FAILURE);

	modified = 0;
	modified |= hashset_add_set(symmetric_difference_set, setA);
	modified |= hashset_remove_set(symmetric_difference_set, setB);
	modified |= hashset_add_set(right_half_set, setB);
	modified |= hashset_remove_set(right_half_set, setA);
	modified |= hashset_add_set(symmetric_difference_set, right_half_set);

	return modified;
}

/*
 * Template to do set operation with an element data
 *
 * @param set
 * @param data
 * @param treeset_function set operation defined in treeset.h
 * @return result of operation
 */
static int
hashset_operation_template_with_data(
	struct hashset_chain *set,
	int data,
	int (*treeset_function)(struct tree_set*, int))
{
	int hash_value, result;
	struct tree_set *chain;

	if (set == NULL) return 0;

	hash_value = hashset_hash_code(data);
	chain 	   = set->table[hash_value];

	result = treeset_function(chain, data);
	hashset_update_size(set);

	return result;
}

/*
 * Template to do set operation on setA using setB.
 * Thus, this is a mutable operation on setA
 *
 * @param operation set operation type defined in SET_OPERATION
 * @param setA
 * @param setB
 * @return result of operation
 */
static int
hashset_operation_template_with_set(
	enum SET_OPERATION operation,
	struct hashset_chain *setA,
	struct hashset_chain *setB)
{
	int result;

	/* Never proceed if inputs are invalid */
	if (setA == NULL || setB == NULL)
		return 0;

	result = hashset_set_operation(operation, setA, setB, NULL, -1);
	hashset_update_size(setA);

	return result;
}

/*
 * Template to do set operation on set using array.
 * Thus, this is a mutable operation on set
 *
 * @param operation set operation type defined in SET_OPERATION
 * @param set
 * @param array_data
 * @param array_size
 * @return result of operation
 */
static int
hashset_operation_template_with_array(
	enum SET_OPERATION operation,
	struct hashset_chain *set,
	int *array_data,
	int  array_size)
{
	int result;
	/* Never proceed if inputs are invalid */
	if (set == NULL		   ||
		array_data == NULL ||
		array_size < 0)
		return 0;

	result = hashset_set_operation(operation, set, NULL, array_data, array_size);
	hashset_update_size(set);

	return result;
}

/*
 * Initialize mutex table for operation with array
 *
 * [NOTE]
 * This function is only called once within the execution of hashset_chain.c
 */
static void
hashset_table_lock_mutex_initialization(void)
{
	int i;

	for (i = 0; i < HASHSET_TABLE_SIZE; i++)
	{
		table_locks[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	}
}

/*
 * Template logic part of thread execution for set operation with set and array.
 * This function is called by operation of both set and array, thus the args have
 * setB and array_data for set operation and array operation respectively.
 * This means that either setB or array_data MUST BE NULL when hashset_set_operation(...)
 * is called as the information NULL is used for other functions called by
 * hashset_set_operation() to understand if the operation is do with set or array
 *
 * @return result of set operation
 */
static int
hashset_set_operation(enum SET_OPERATION operation,
					  struct hashset_chain *setA,
					  struct hashset_chain *setB,
					  int *array_data,
					  int  array_size)
{
	int i, error, success, num_threads;
	pthread_t *tid;
	struct task_data *data;
	struct thread_task *tasks;

	num_threads = hashset_compute_proper_number_of_threads(setB, array_size);
	/* Set up pthreads, data for task, tasks, and pthread_once for array operations */
	tid = (pthread_t *)calloc(num_threads, sizeof(pthread_t));
	if (tid == NULL) {
		perror("Failed to allocate pthread memory");
		success = 0;
		goto end;
	}
	data = (struct task_data*)malloc(sizeof(struct task_data));
	if (data == NULL) {
		perror("Failed to allocate data memory");
		success = 0;
		goto free_tid_end;
	}
	tasks = (struct thread_task*)calloc(num_threads, sizeof(struct thread_task));
	if (tasks == NULL) {
		perror("Failed to allocate task memory");
		success = 0;
		goto free_tid_data;
	}
	error = pthread_once(&atmostonece_for_table_lock_init, hashset_table_lock_mutex_initialization);
	if (error) {
		perror("Failed to initialize table locks for array operation");
		success = 0;
		goto end;
	}

	/* Set up task data */
	hashset_setup_task_data(data, operation, setA, setB, array_data, array_size);

	/* Create threads as many threads as being set up */
	hashset_create_thread(tid, data, tasks, num_threads);

	/* Wait for thread execution */
	for (i = 0; i < num_threads; i++)
	{
		if (pthread_equal(pthread_self(), tid[i]))
			continue;

		error = pthread_join(tid[i], NULL);
		if (error)
			perror("failed to join thread");
	}

	success = 1;
	for (i = 0; i < num_threads; i++)
	{
		success &= tasks[i].success; // 1 if succeeded, 0 otherwise;
	}

	free(tasks);

free_tid_data:
	free(data);

free_tid_end:
	free(tid);

end:
	return success;
}

/*
 * Create thread (and yes invoke it!)
 */
static int
hashset_create_thread(pthread_t *tid,
					  struct task_data *data,
					  struct thread_task *tasks,
					  int num_threads)
{
	int size, task_left, threads_left, task_size_per_thread, i, next_index, from, to, error;

	size = (data->array_data == NULL) ? HASHSET_TABLE_SIZE : data->array_size;
	// task_size_per_thread = size / num_threads;

	next_index = 0;
	for (i = 0; i < num_threads; i++)
	{
		task_left    = size - next_index;
		threads_left = num_threads - i;
		task_size_per_thread = task_left / threads_left;

		/* Range of table or array index to do operation for one thread */
		from = next_index;
		to   = (i == num_threads-1) ? size : next_index + task_size_per_thread;

		/* Create task with proper set operation */
		hashset_setup_thread_task(data, &(tasks[i]), from, to);

		/* Create threads */
		error = pthread_create(tid + i, NULL, &hashset_thread_operation, &tasks[i]);
		if (error) {
			// FIXME(mas): In case of failure, we might retry to create pthread 
			tid[i] = pthread_self();
		}

		/* Set the next _from_ index */
		next_index += task_size_per_thread;
	}

	return 1;
}

/*
 * Compute the proper number of threads in case NUM_THREAD is greater than
 * array_size or table size as there's no performance increase but a lot of overhead.
 */
static int
hashset_compute_proper_number_of_threads(struct hashset_chain *setB,
										 int array_size)
{
	int num_threads;

	if (setB == NULL) // Indicates we'll do operation with array_data
		num_threads = (array_size < NUM_THREADS) ? array_size : NUM_THREADS;
	else
		num_threads = (HASHSET_TABLE_SIZE < NUM_THREADS) ? HASHSET_TABLE_SIZE : NUM_THREADS;

	/*FIXME(mas): This condition might be unnecessary */
	if (num_threads < 0) // Invalid operation, so just return 0 and will terminate soon.
		return 0;

	return num_threads;
}

static void
hashset_setup_task_data(struct task_data *data,
						enum SET_OPERATION operation,
						struct hashset_chain *setA,
						struct hashset_chain *setB,
						int *array_data,
						int  array_size)
{
	data->operation = operation;
	data->setA = setA;
	data->setB = setB;
	data->array_data = array_data;
	data->array_size = array_size;
}

/*
 * This is the part you create an object that is used when calling Callable()
 */
static void
hashset_setup_thread_task(
	struct task_data *data,
	struct thread_task *task,
	int from,
	int to)
{
	enum SET_OPERATION operation;

	task->data = data;
	task->from = from;
	task->to   = to;

	/*
	 * Set proper set operation.
	 * Operations are from the tree library you use,
	 * in our case *treeset*
	 *
	 * function_with_data() is used for set operation with one input
	 * function_with_chain() is used for set operation with a chain which is a treeset.
	 */
	operation = data->operation;
	switch(operation) {
		case ADD:
			task->function_with_data  = &treeset_add;
			task->function_with_chain = &treeset_add_set;
			break;
		case REMOVE:
			task->function_with_data  = &treeset_remove;
			task->function_with_chain = &treeset_remove_set;
			break;
		case RETAIN:
			/* task->function_with_data  = NO NEED */
			task->function_with_chain = &treeset_retain_set;
			break;
		case FIND:
			task->function_with_data  = &treeset_find;
			task->function_with_chain = &treeset_find_set;
			break;
		default:
			;
	}
}

/*
 * Execute the expected set operation. i.e. with set or array
 */
static void *
hashset_thread_operation(void *arg)
{	
	struct thread_task *task;
	task = (struct thread_task *)arg;

	if (task->data->setB) // Indicates that we'll operate with set, NOT ARRAY.
		hashset_operate_with_all_elements_of_set(task);
	else
		hashset_operate_with_all_elements_of_array(task);

	return NULL;
}

/*
 * Actual set operation by one thread. Each thread has its own task doing the operation
 * between table[i] to table[j] of setA and setB. They do set operation using
 * table[i] of setA and setB where from <= i < to.
 * This is correct as, for example, table[i] of setA and setB has elements whose hash values
 * are the same. So any elements in the table[i] simply belong to table[i] of ANY set based on
 * our definition.
 *
 * [NOTE]
 * The range of the operation for hash table is determined in hashset_create_thread(...)
 */
static void
hashset_operate_with_all_elements_of_set(struct thread_task *task)
{
	int i, from, to, success_bit;
	struct task_data *data;
	struct hashset_chain *setA, *setB;
	struct tree_set *chainA, *chainB;
	int (*treeset_function)(struct tree_set *, struct tree_set *);

	from = task->from;
	to   = task->to;
	data = task->data;
	setA = data->setA;
	setB = data->setB;
	treeset_function = task->function_with_chain;

	success_bit = 1;
	for (i=from; i < to; i++) {
		chainA = setA->table[i];
		chainB = setB->table[i];
		success_bit &= treeset_function(chainA, chainB);
	}

	task->success = success_bit; //check if all treeset_function operation succeeded.
}

/*
 * Actual set operation by one thread. Each thread has its own task doing the operation
 * between array[from] and array[to]. They iterate through from [from] to [to], and 
 * do set operation for each element of the array with _lock_.
 *
 * [NOTE]
 * The range of the operation for array is determined in hashset_create_thread(...)
 */
static void
hashset_operate_with_all_elements_of_array(struct thread_task *task)
{
	int i, from, to, hash_value, d, *array_data, success_bit, error;
	struct task_data *data;
	struct hashset_chain *set;
	struct tree_set *chain;
	int (*treeset_function)(struct tree_set *, int);

	from = task->from;
	to   = task->to;
	data = task->data;
	set  = data->setA;
	array_data = data->array_data;
	treeset_function = task->function_with_data;

	success_bit = 1;
	for (i = from; i < to; ++i)
	{
		d = array_data[i];
		hash_value = hashset_hash_code(d);

		/* mutex lock for set->table[hash_value] */
		error = pthread_mutex_lock(&(table_locks[hash_value]));
		if (error) {
			task->success = 0;
			return;
		}

		/*** CRITICAL SECTION ****/
		chain = set->table[hash_value];
		success_bit &= treeset_function(chain, d);
		/*** CRITICAL SECTION ****/

		/* free lock for set->table[hash_value] */
		error = pthread_mutex_unlock(&(table_locks[hash_value]));
		if (error) {
			task->success = 0;
			return;
		}
	}

	task->success = success_bit;
}
