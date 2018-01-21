#ifndef __HEAP__
#define __HEAP__

#include "header.h"

/* Global Variables */
extern int h1NumOfEntries, h2NumOfEntries, bucketSize, connectionSize;

typedef struct heapnode_t {
	int id;			// for debugging - printing
	char* number;
	double revenue;
	struct heapnode_t * left;
	struct heapnode_t * right;
} HeapNode;

typedef struct heap_t {
	HeapNode* root;
	int nodes;
	double total_revenue;
	double*** charges;	// a pointer to the charging table, created by the infos given to us by our configuration file
} Heap;
/* for Heap */
void initialize_heap(Heap*, double***);
void clear_heap(Heap*);
double*** take_charges(Heap*);
void print_heap(Heap*);
void load_heap_to_array(Heap*, HeapNode***, int*);
void recursively_delete(HeapNode*);	// assistant function for when "clearing" the heap
int is_heap_empty(Heap*);
int insert_to_heap(Heap*, HeapNode*);
int try_to_fit_to_heap(HeapNode*, HeapNode*, char***, int*); // to insert in case that we already have node's number in heap
void max_heapify(Heap*, char**, int);
int top_k_percent_revenue(Heap*, double);
/* for HeapNode */
HeapNode* initialize_heapnode(Heap*, char*, int, int, int);
void clear_heapnode(HeapNode*);
void print_heapnode(HeapNode*);
void load_heapnode_to_array(HeapNode*, HeapNode***, int*);

#endif