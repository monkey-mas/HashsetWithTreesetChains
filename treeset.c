// Copyright (c) 2015 Masaru Nomura
// Released under the MIT license
// http://opensource.org/licenses/mit-license.php

#include <stdlib.h>
#include <stdio.h>

#include "treeset.h"


static struct avlnode *treeset_create_avlnode(int data);
static void treeset_free_avlnode(struct avlnode *node);
static void treeset_free_tree(struct avlnode *root);
static struct avlnode *treeset_rotate_right(struct avlnode *root);
static struct avlnode *treeset_rotate_left(struct avlnode *root);
static struct avlnode *treeset_double_rotate_left_right(struct avlnode *root);
static struct avlnode *treeset_double_rotate_right_left(struct avlnode *root);
static struct avlnode *treeset_balance(struct avlnode *root);
static struct avlnode *treeset_balance_right_subtree(struct avlnode *root);
static int  treeset_to_array_rec(struct avlnode *node, int *array, int size, int count);
static void treeset_increment_size_by(struct tree_set *set, int diff);
static void treeset_decrement_size_by(struct tree_set *set, int diff);
static int  treeset_count_height(struct avlnode *tree);
static void treeset_update_height(struct avlnode *tree);
static struct avlnode *treeset_erase_data(struct avlnode *root, int data, struct operation_result *result);
static struct avlnode *treeset_insert_data(struct avlnode *root, int data, struct operation_result *result);
static int  treeset_find_data(struct avlnode *root, int data);
static int  treeset_binary_search_array(int *array, int data, int from, int to);
static void treeset_merge_sort_array(int *array, int  array_size, int *buff);


struct tree_set*
treeset_create_set()
{
	struct tree_set *set;
	set = (struct tree_set *)malloc(sizeof(struct tree_set));
	if (set == NULL) {
		return NULL;
	}

	/* Initialize size & tree */
	set->size = 0;
	set->tree = NULL;

	return set;
};

/*
 * Free treeset and nodes
 */
void
treeset_free_set(struct tree_set *set)
{
	struct avlnode *tree;

	if (set == NULL)
		return;

	if (set->size == 0)
		goto end;

	/* Free tree if necessary */
	tree = set->tree;
	treeset_free_tree(tree);
	set->size = 0;

end:
	free(set);
}

/*
 * Compute the height(rank) of tree.
 * Node itself is considered as height 1.
 * Time complexity: O(1)
 */
static int
treeset_count_height(struct avlnode *tree)
{
	int left_height, right_height, current_height;

	left_height  = (tree->lch == NULL) ? 0 : tree->lch->height;
	right_height = (tree->rch == NULL) ? 0 : tree->rch->height;
	current_height = ((left_height < right_height) ? right_height : left_height) + 1;

	return current_height;
}

static void
treeset_update_height(struct avlnode *tree)
{
	tree->height = treeset_count_height(tree);
}

/*
 * Add an element data to set
 *
 * @return 1 if a new data was inserted 0 otherwise
 */
int
treeset_add(struct tree_set *set, int data)
{
	struct operation_result result;
	struct avlnode *root, *new_root;

	if (set == NULL) {
		result.modified = 0;
		goto end;
	}

	root 	  = set->tree;
	new_root  = treeset_insert_data(root, data, &result);
	set->tree = new_root;

	if (result.modified)
		treeset_increment_size_by(set, 1);

end:
	return result.modified;
}
/*
 * Add all elements in setB to setA
 * @return 1 if the set is modified due to the operation, 0 otherwise.
 */
int
treeset_add_set(struct tree_set *setA,
				struct tree_set *setB)
{
	int sizeB, *arrayB, modified;

	if (setB == NULL) return 0;

	sizeB = setB->size;
	arrayB = (int *)calloc(sizeB, sizeof(int));
	treeset_to_array(setB, arrayB, sizeB);
	modified = treeset_add_array(setA, arrayB, sizeB);

	free(arrayB);
	return modified;
}

/*
 * Add array to set
 * @return 1 if the set is modified due to the operation, 0 otherwise.
 * 
 * !!!Careful!!!
 * The function DOES NOT guarantee correct operations if array_data and array_size are not given properly.
 * Such as segmentation fault due to invalid memory access caused by wrong input array size
 */
int
treeset_add_array(
	struct tree_set *set,
	int *array_data,
	int  array_size)
{
	int i, modified;

	/* Don't proceed if inputs are _abviously_ invalid */
	if (set == NULL 	   ||
		array_data == NULL ||
		array_size < 0 )
		return 0;

	modified = 0;
	for (i = 0; i < array_size; ++i)
	{
		int data = *(array_data + i);
		modified |= treeset_add(set, data);
	}

	return modified;
}

/*
 * Remove an element data from set
 *
 * @return 1 if a data was removed 0 otherwise
 */
int
treeset_remove(struct tree_set *set, int data)
{
	struct operation_result result;
	struct avlnode *root, *new_root;

	if (set == NULL) {
		result.modified = 0;
		goto end;
	}

	root 	  = set->tree;
	new_root  = treeset_erase_data(root, data, &result);
	set->tree = new_root;

	if (result.modified)
		treeset_decrement_size_by(set, 1);

end:
	return result.modified;
}

/*
 * Remove all elements in setB from setA
 *
 * @return 1 if the set is modified due to the operation, 0 otherwise.
 */
int
treeset_remove_set(struct tree_set *setA,
				   struct tree_set *setB)
{
	int sizeB, *arrayB;

	if (setB == NULL) return 0;

	sizeB = setB->size;
	arrayB = (int *)calloc(sizeB, sizeof(int));
	treeset_to_array(setB, arrayB, sizeB);
	treeset_remove_array(setA, arrayB, sizeB);

	free(arrayB);
	return 1;
}

/*
 * Remove all elements in array from set
 *
 * @return 1 if the set is modified due to the operation, 0 otherwise.
 *
 * !!!Careful!!!
 * The function DOES NOT guarantee correct operations if array_data and array_size are not given properly.
 * Such as segmentation fault due to invalid memory access caused by wrong input array size
 */
int
treeset_remove_array(
	struct tree_set *set,
	int *array_data,
	int  array_size)
{
	int i;

	/* Don't proceed if inputs are _abviously_ invalid */
	if (set == NULL 	   ||
		array_data == NULL ||
		array_size < 0)
		return 0;

	for (i = 0; i < array_size; ++i)
	{
		int data = *(array_data + i);
		treeset_remove(set, data);
	}

	return 1;
}

/*
 * Intersection operation for setA and setB
 * In the end, setA only has intersection of setB
 *
 * @return 1 if operation succeeded 0 otherwise
 */
int
treeset_retain_set(struct tree_set *setA,
				   struct tree_set *setB)
{
	int sizeB, *arrayB, result;

	if (setB == NULL) return 0;

	sizeB = setB->size;
	arrayB = (int *)calloc(sizeB, sizeof(int)); // allocation check will done in tree_set_array(...)
	treeset_to_array(setB, arrayB, sizeB);
	result = treeset_retain_array(setA, arrayB, sizeB);

	free(arrayB);
	return result;
}

/*
 * Intersection operation for set and array elements
 * In the end, setA only has intersection of array
 *
 * @return 1 if operation succeeded 0 otherwise
 *
 * !!!Careful!!!
 * The function DOES NOT guarantee correct operations if array_data and array_size are not given properly.
 * Such as segmentation fault due to invalid memory access caused by wrong input array size
 *
 * TODO(mas): Clean up and divide the code into smaller tasks.
 */
int
treeset_retain_array(
	struct tree_set *setA,
	int *arrayB,
	int  sizeB)
{
	int i, data, sizeA, found, max_size, count;
	int *arrayA, *retain, *buff;

	/* Don't proceed if inputs are _abviously_ invalid */
	if (setA == NULL   ||
		arrayB == NULL ||
		sizeB < 0 )
		return 0;

	/* Create temporal array of setA elements */
	sizeA = setA->size;
	arrayA = (int *)calloc(sizeA, sizeof(int)); // allocation check will done in tree_set_array
	treeset_to_array(setA, arrayA, sizeA);

	/* Collect and store retain elements of arrayA and arrayB temporarily */
	count = 0;
	max_size = (sizeA < sizeB) ? sizeA : sizeB;
	retain = (int *)calloc(max_size, sizeof(int));
	if (retain == NULL)
		return 0;

	for (i = 0; i < sizeB; i++)
	{
		data  = *(arrayB + i);
		found = treeset_binary_search_array(arrayA, data, 0, sizeA-1);
		if (found)
			retain[count++] = data;
	}

	/* Remove elements that are not in retain(intersection) from setA */
	// merge_sort retain as the arrayB might be unsorted input.
	buff = (int *)calloc(count, sizeof(int));
	if (buff == NULL)
		return 0;
	treeset_merge_sort_array(retain, count, buff);

	for (i = 0; i < sizeA; i++)
	{
		data  = *(arrayA + i);
		found = treeset_binary_search_array(retain, data, 0, count-1);
		if (!found) 
			treeset_remove(setA, data);
	}

	free(arrayA);
	free(retain);
	free(buff);
	return 1;
}

/*
 * Time complexity: O( lg(N) )
 * Space complexity: O( lg(N) )
 *
 * @return 1 if set contains data, otherwise 0.
 */
int
treeset_find(struct tree_set *set, int data)
{
	int found;
	struct avlnode *tree;

	tree  = set->tree;
	found = treeset_find_data(tree, data);

	return found;
}

/*
 * Shallow copy data in set to array_data
 *
 * @param set
 * @param array_data array to store node data
 * @param array_size maximum number of elements the array can store
 *
 * !!!Careful!!!
 * The function DOES NOT guarantee correct operations if array and its size are not given properly.
 * Such as segmentation fault due to invalid memory access caused by wrong input array size
 */
void
treeset_to_array(
	struct tree_set *set,
	int *array_data,
	int  array_size)
{
	struct avlnode *tree;
	int count;

	if (set == NULL)
		return;

	tree = set->tree;
	count = 0;
	treeset_to_array_rec(tree, array_data, array_size, count);
}

/*
 * Function to recursively search for all nodes in node and its left & right subtree
 * [NOTE]
 * The function stops if the total number of nodes exceeds array size
 * so that unexpected memory access can be avoided iff array_size is set correctly.
 *
 * @param node
 * @param array_data array to store node data
 * @param array_size
 * @param count number to count node in left or right subtree
 * @return the total number of subtree nodes that indicates the next index to store data into array
 */
static int
treeset_to_array_rec(
	struct avlnode *node,
	int *array_data,
	int  array_size,
	int  count)
{
	int current_size, next_size;
	/* Don't proceed in case inputs are invalid */
	if (node == NULL	   ||
		array_data == NULL ||
		array_size < 0	   ||
		array_size < count )
		return count;

	current_size = treeset_to_array_rec(node->lch, array_data, array_size, count);

	if (current_size >= array_size)
		return current_size;

	*(array_data + current_size) = node->data;

	next_size = treeset_to_array_rec(node->rch, array_data, array_size, current_size+1);

	return next_size;
}

static void
treeset_increment_size_by(struct tree_set *set, int diff)
{
	set->size += diff;
}

static void
treeset_decrement_size_by(struct tree_set *set, int diff)
{
	set->size -= diff;
}

static struct avlnode *
treeset_create_avlnode(int data)
{
	struct avlnode *node;

	node = (struct avlnode *)malloc(sizeof(struct avlnode));
	if (node == NULL) {
		return NULL;
	}

	/* Initial set up */
	node->data = data;
	node->height = 1;
	node->lch = NULL;
	node->rch = NULL;

	return node;
}

/*
 * Free node assuming that 
 * left and right children are already freed, removed or saved.
 */
static void
treeset_free_avlnode(struct avlnode *node)
{
	if (node == NULL) {
		return;
	}

	/*clean node information*/
	node->data = 0;
	node->height = 1;
	node->lch = NULL;
	node->rch = NULL;

	free(node);
}

/*
 * Free all nodes of a tree starting from root
 * in a depth-fast-search manner.
 *
 * Time complexity: O(N)
 * Space compexity: O(lg(N))
 * where N is the number of all nodes
 */
static void
treeset_free_tree(struct avlnode *root)
{
	if (root == NULL) {
		return;
	}

	if (root->lch != NULL)
		treeset_free_tree(root->lch);
	if (root->rch != NULL)
		treeset_free_tree(root->rch);

	treeset_free_avlnode(root);
}

/*
 * Rotate root into right direction.
 * Time complexity: O(1)
 */
static struct avlnode *
treeset_rotate_right(struct avlnode *root)
{
	struct avlnode *new_root;

	if (root == NULL)
		return NULL;

	new_root = root->lch;
	if (new_root == NULL)
		return root;

	// Right Rotation
	root->lch = new_root->rch;
	new_root->rch = root;

	// Re-calculate heights of root->rch and root
	treeset_update_height(new_root->rch);
	treeset_update_height(new_root);

	return new_root;
}

/*
 * Rotate root into left direction.
 * Time complexity: O(1)
 */
static struct avlnode *
treeset_rotate_left(struct avlnode *root)
{
	struct avlnode *new_root;

	if (root == NULL)
		return NULL;

	new_root = root->rch;
	if (new_root == NULL)
		return root;

	// Left Rotation
	root->rch = new_root->lch;
	new_root->lch = root;

	// Re-calculate heights of root.lch and root
	treeset_update_height(new_root->lch);
	treeset_update_height(new_root);

	return new_root;
}

/*
 * Double-rotate root
 * First rotate left child of root into left direction then right.
 * Time complexity: O(1) 
 */
static struct avlnode *
treeset_double_rotate_left_right(struct avlnode *root)
{
	struct avlnode *lch, *new_root;

	// Rotate left child and set as new left;
	lch = root->lch;
	if (lch != NULL)
		root->lch = treeset_rotate_left(lch);

	// Rotate root into right direction
	new_root = treeset_rotate_right(root);
	return new_root;
}

/*
 * Double-rotate root assuming that right child is not NULL.
 * First rotate right child of root into right direction then left.
 * Time complexity: O(1) 
 */
static struct avlnode *
treeset_double_rotate_right_left(struct avlnode *root)
{
	struct avlnode *rch, *new_root;

	// Rotate right child and set as new right; 
	rch = root->rch;
	if (rch != NULL)
		root->rch = treeset_rotate_right(rch);

	// Rotate root into left direction
	new_root = treeset_rotate_left(root);
	return new_root;
}

/*
 * Balace a subtree starting from the root.
 *
 * Time complexity: O( lg(N) )
 */
static struct avlnode *
treeset_balance(struct avlnode *root)
{
	// balance tree by rotating once or twice
	int lch_height, rch_height, grch_height, glch_height, balance_degree;

	rch_height = (root->rch == NULL) ? 0 : root->rch->height;
	lch_height = (root->lch == NULL) ? 0 : root->lch->height;

	balance_degree = rch_height - lch_height;
	if (balance_degree == 2) { //rch is higher than lch
		/* TODO(massa): Clean up code and add comments */
		if (root->lch == NULL)
			return treeset_double_rotate_right_left(root);
		else if (root->rch->rch == NULL)
			return treeset_double_rotate_right_left(root);
		else if (root->rch->lch == NULL)
			return treeset_rotate_left(root);
		else {
			/*
			 * As we can't tell if we rotate right or left,
			 * we need to search for the grand children of rihgt child
			 */
			grch_height = (root->rch->rch == NULL) ? 0 : root->rch->rch->height;
			glch_height = (root->rch->lch == NULL) ? 0 : root->rch->lch->height;
			balance_degree = grch_height - glch_height;
			
			if (balance_degree == 1)
				return treeset_rotate_left(root);
			else if (balance_degree == -1)
				return treeset_double_rotate_right_left(root);
		}
	}
	else if (balance_degree == -2) { // lch is higher than rch
		if (root->rch == NULL)
			return treeset_double_rotate_left_right(root);
		else if (root->lch->lch == NULL)
			return treeset_double_rotate_left_right(root);
		else if (root->lch->rch == NULL)
			return treeset_rotate_right(root);
		else {
			/*
			 * As we can't tell if we rotate right or left,
			 * we need to search for the grand children of rihgt child
			 */
			grch_height = (root->lch->rch == NULL) ? 0 : root->lch->rch->height;
			glch_height = (root->lch->lch == NULL) ? 0 : root->lch->lch->height;
			balance_degree = grch_height - glch_height;

			if (balance_degree == -1) // additional node exists in self.lch.lch subtree
				return treeset_rotate_right(root);
			else if (balance_degree == 1)
				return treeset_double_rotate_left_right(root);
		}
	}

	/* 
	 * if the balance degree is between -1 and 1 inclusive,
     * there is no need for rotation. So just return root.
     */
	return root;
}

/*
 * @return 1 if avltree contains data, otherwise 0.
 */
static int
treeset_find_data(struct avlnode *root, int data)
{
	/* Not found */
	if (root == NULL)
		return 0;

	/* Found */
	if (root->data == data)
		return 1;

	/* Continue to search for data */
	if (root->data < data)
		return treeset_find_data(root->rch, data);
	else
		return treeset_find_data(root->lch, data);
}

/*
 * Check if all elements in setB exist in setA
 *
 * @return 1 if true 0 otherwise
 */
int
treeset_find_set(struct tree_set *setA,
				 struct tree_set *setB)
{
	int sizeB, *arrayB, found_all;

	if (setB == NULL) return 0;

	sizeB = setB->size;
	arrayB = (int *)calloc(sizeB, sizeof(int));

	treeset_to_array(setB, arrayB, sizeB);
	found_all = treeset_find_array(setA, arrayB, sizeB);

	free(arrayB);
	return found_all;
}

/*
 * Check if all elements in array exist in set
 *
 * @return 1 if true 0 otherwise
 *
 * !!!Careful!!!
 * The function DOES NOT guarantee correct operations if array_data and array_size are not given properly.
 * Such as segmentation fault due to invalid memory access caused by wrong input array size
 */
int
treeset_find_array(
	struct tree_set *set,
	int *array_data,
	int  array_size)
{
	int i, data, found;

	/* Don't proceed if inputs are _abviously_ invalid */
	if (set == NULL 	   ||
		array_data == NULL ||
		array_size < 0 )
		return 0;

	for (i = 0; i < array_size; ++i)
	{
		data  = *(array_data + i);
		found = treeset_find(set, data);
		if (!found)
			return 0;
	}
	return 1;
}

/*
 * Binary-search array for data
 * This func assumes taht array is already sorted in accending order
 *
 * @param array
 * @param data element to search for
 * @param from begin index range(0-based inclusive)
 * @param to   end index range(0-based inclusive)
 * @return 1 if found, otherwise 0
 */
static int
treeset_binary_search_array(
	int *array,
	int  data,
	int  from,
	int  to)
{
	int mid, d;

	if (array == NULL || from > to)
		return 0;

	mid = (from + to) / 2;
	d   = array[mid];

	if (d == data)
		return 1;
	else if (d < data)
		return treeset_binary_search_array(array, data, mid+1, to);
	else //  d > data : so keep searching left half
		return treeset_binary_search_array(array, data, from, mid-1);
}

/*
 * Merge-sort array for data in accending order
 *
 * buff is prepared by the orignal caller function
 * so that we can gain memory efficiency
 *
 * Time complexity: O( lg(N) )
 * Space complexity: O( lg(N) )
 * where N is the array size
 */
static void
treeset_merge_sort_array(
	int *array,
	int  array_size,
	int *buff)
{
	int i, j, k, mid;

	if (array_size < 2)
		return;

	/* Halve array */
	mid = array_size / 2;
	treeset_merge_sort_array(array      , mid       	, buff);
	treeset_merge_sort_array(array + mid, array_size-mid, buff);

	/* Merge halves */
	for (i = 0; i < array_size; i++)
		buff[i] = array[i];

	i = k = 0;
	j    = mid;
	while (i < mid && j < array_size) {
		if (buff[i] < array[j])
			array[k++] = buff[i++];
		else
			array[k++] = array[j++];
	}

	while (i < mid) {
		array[k++] = buff[i++];
	}
}

/*
 * Insert data into binary tree
 *
 * Time complexity: O( lg(N) )
 * Space complexity: O( lg(N) )
 */
static struct avlnode *
treeset_insert_data(
	struct avlnode *root,
	int data,
	struct operation_result *result)
{
	struct avlnode *new_root;

	if (root == NULL) {
	/*
	 * IMO(massa): Add functionality of retry to crate_node
	 * Even we assume that malloc(...) might fail,
	 * there's not funcitonality to retry when this happens.
	 * For _library users_, we need to care about it by:
	 *    Allowing users to determine max retry times;
	 *    Logging or printing out modified msgs when retry even failed.
	 */
	 	new_root = treeset_create_avlnode(data);
	 	/* For debugging usage its goot to check the return value of avltree_create_node(...) :)
		 * we can do like
		 * if (new_root)
		 *		goto end;
		 * else
		 	modified = 0;
		 *    fprint(stderr, ...);
		 * 
		modified |=  */
		result->modified = 1; // A new elem is inserted
	 	goto end;
	}
	else if (root->data > data) {
		root->lch = treeset_insert_data(root->lch, data, result);
	}
	else if (root->data < data) {
		root->rch = treeset_insert_data(root->rch, data, result);
	}
	else { // if (root->data == data)
		result->modified = 0; // Nothing is inserted as we already have the same data in set
		return root;
	}

	// Balance tree(root) by rotating once or twice after re-compute its height
	root->height = treeset_count_height(root);
	new_root = treeset_balance(root);

end:
	return new_root;
}

/*
 * Delete data from a tree.
 * If a node with the data is not found, then no modification is done to the tree.
 *
 * If there exist multiple nodes whose value is equal to data,
 * then one that found first is deleted.
 * If this is the case, the node is replaced with another node with the following manner.
 * 1) If left child of the node does not exist, then its right child is replaced.  
 * 2) If the node has left child but not grand child (child of left node),
 *    left child is replaced.
 * 3) If the node has left child and grand child(child of left node),
 *    find the right-most leaf which is the decendent of left child and repalce it.
 *
 * Time complexity: O( lg(N) )
 * Space complexity: O( lg(N) )
 */
static struct avlnode *
treeset_erase_data(
	struct avlnode *root,
	int data,
	struct operation_result *result)
{
	struct avlnode *lch, *rch, *head, *right_most;

	/* data to be deleted not found */
	if (root == NULL) {
		result->modified = 0;
		return NULL;
	}
	
	if (root->data > data) {
		root->lch = treeset_erase_data(root->lch, data, result);
		return root;
	}
	else if (root->data < data) {
		root->rch = treeset_erase_data(root->rch, data, result);
		return root;
	}
	/* 
	 * found data to be deleted.
	 * so we do one of the three operations.
	 */
	else {
		result->modified = 1;

		if (root->lch == NULL) { // left child does not exist
			rch = root->rch;
			treeset_free_avlnode(root);

			return rch;
		}
		else if (root->lch->rch == NULL) {
			// left child does not have its right child
			lch = root->lch;
			lch->rch = root->rch;
			treeset_free_avlnode(root);

			return lch;	
		}
		else {
			/* Find the right-most node that is the new root &
			 * replace the node with its left child
		 	 *
			 *     x                   x
			 *       \                  \
			 *      right_most   =>      lch
			 *       /
			 *     lch
			 */
			head = root->lch;
			while (head->rch->rch != NULL)
				head = head->rch;
			right_most = head->rch;
			head->rch = head->rch->lch;

			/* Balance the subtree of right child repeatedly. */
			root->lch = treeset_balance_right_subtree(root->lch);
			right_most->rch = root->rch;
			right_most->lch = root->lch;

			free(root);

			/* Balance tree */
			treeset_update_height(right_most);
			right_most = treeset_balance(right_most);

			return right_most;
		}
	}
}

/*
 * Recursively balance the all right subtree starting from right-most node of root
 */
static struct avlnode *
treeset_balance_right_subtree(struct avlnode *root)
{
	/* Stop recursion */
	if (root == NULL)
		return NULL;
	/* Continue recursion on right child */
	else if (root->rch != NULL)
		root->rch = treeset_balance_right_subtree(root->rch);

	/*
	 * Balance tree:
	 * As rotation function used in balance(..) computes height as well,
	 * we need to compute height of current root first before balancing it.
	 */
	treeset_update_height(root);
	root = treeset_balance(root);

	return root;
}
