// Copyright (c) 2015 Masaru Nomura
// Released under the MIT license
// http://opensource.org/licenses/mit-license.php

#ifndef TREE_SET_H
#define TREE_SET_H

struct tree_set {
	int size; /* total number of elements in set */
	struct avlnode *tree;
};

struct avlnode {
	int data;
	int height;
	struct avlnode *lch; /*left child*/
	struct avlnode *rch; /*right child*/
};

/*
 * Used to know add/remove operations
 * For example, if we try to add/remove an element that already in set,
 * then modified == 0 indicating that we actually did NOTHING
 *
 * When we add/remove elements that are in set,
 * then modified == 1
 *
 * By implementing this, we get the info about whether or not elements
 * are inserted/deleted and can easilly update the current size of set as well.
 */
struct operation_result {
	int modified;
};

struct tree_set* treeset_create_set();
void treeset_free_set(struct tree_set *set);
int  treeset_add(struct tree_set *set, int data);
int  treeset_add_set(struct tree_set *setA, struct tree_set *setB);
int  treeset_add_array(struct tree_set *setA, int *array_data, int array_size);
int  treeset_remove(struct tree_set *set, int data);
int  treeset_remove_set(struct tree_set *setA, struct tree_set *setB);
int  treeset_remove_array(struct tree_set *set, int *array_data, int array_size);
int  treeset_find(struct tree_set *set, int data);
int  treeset_find_set(struct tree_set *set, struct tree_set *setB);
int  treeset_find_array(struct tree_set *set, int *array_data, int array_size);
int  treeset_retain_set(struct tree_set *set, struct tree_set *setB);
int  treeset_retain_array(struct tree_set *set, int *array_data, int array_size);
void treeset_to_array(struct tree_set *set, int *array_data, int  array_size);

#endif