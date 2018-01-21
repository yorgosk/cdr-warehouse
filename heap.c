#include "heap.h"

void initialize_heap(Heap* h, double*** charges) {
	h->root = NULL;
	h->nodes = 0;
	h->total_revenue = 0;
	h->charges = charges;
}

void clear_heap(Heap* h) {
	if (!h) return;	// can't clear a non-existent heap
	
	recursively_delete(h->root);
	h->nodes = 0;	// we reached here, so we have successfully cleared the heap
}

double*** take_charges(Heap* h) {
	if (!h) return NULL;	// can't take the charging from a non-existent heap
	
	return h->charges;
}

void print_heap(Heap* h) {
	if (!h) return;	// can't print a non-existent heap
	
	printf("Our heap has %d nodes\n", h->nodes);
	print_heapnode(h->root);
}

void load_heap_to_array(Heap* h, HeapNode*** clients, int* size) {
	if (!h || !clients) return;	// both have to exist
	
	load_heapnode_to_array(h->root, clients, size);
}

// assistant function for when "clearing" the heap
void recursively_delete(HeapNode* hn) {
	if (hn == NULL) return;
	recursively_delete(hn->left);		// recursively delete node's left subtree
	recursively_delete(hn->right);	// recursively delete node's right subtree
	clear_heapnode(hn);		// then delete node
}

int is_heap_empty(Heap* h) {
	if (!h) return -1;	// heap has to exist 

	return h->nodes == 0;		// if there are no nodes, we have an empty heap
}

int insert_to_heap(Heap* h, HeapNode* node) {
	if (!h) return -1;	// heap has to exist 
	
	h->total_revenue += node->revenue;	// count total revenue in heap

	if (is_heap_empty(h)) {	// if our heap is empty
		h->root = node;	// we must insert at root
		h->nodes++;		// one more heapnode
		return 0;
	}

	// if our heap is not empty
	int i, length = 0;		// the length of our traversal path (and the size of the below "path" array)
	char **path = NULL;	// we will need to store our traversal path
	
	int ret = try_to_fit_to_heap(h->root, node, &path, &length);	// if we already have node's number in heap
	if (ret == 1) {
		max_heapify(h, path, length);	// keep the heap's order post-insertion
		for (i = 0; i < length; i++) free(path[i]);
		free(path);
		return 0;	// we are done inserting
	}

	// if node's number not in heap
	// I followed the algorithms presented at: https://www.cpp.edu/~ftang/courses/CS241/notes/Building_Heaps_With_Pointers.pdf (a classmate posted it on piazza)
	int newnodes = h->nodes+1;	// our nodes post-insertion
	int current_level = floor(log(newnodes)/log(2));	// we want base-2 logarithm
	int max_nodes_in_level = pow(2, current_level);			// maximum number of nodes in our current level
	int max_nodes_in_tree = pow(2, current_level+1) - 1;	// maximum number of nodes our tree can currently contain
	int horpos = max_nodes_in_level - (max_nodes_in_tree - newnodes);	// our node's position horizontaly
	
	while (current_level > 0) {
		path = realloc(path, (length+1)*sizeof(char*));
		if (horpos % 2 == 0) {
			path[length] = malloc(strlen("right")+1);
			strcpy(path[length], "right");
		} else {
			path[length] = malloc(strlen("left")+1);
			strcpy(path[length], "left");
		}
		length++;

		double percent = (double)horpos / (double)max_nodes_in_level;	// how much "through the level" we are horizontaly
		max_nodes_in_level = max_nodes_in_level / 2;	// we are going one level up
		horpos = ceil(percent*max_nodes_in_level);
		current_level--;	// we are going one level up
	}

	HeapNode *temp = h->root;	// temporary pointer is pointing to where we are going to insert (initially, pro heapification)
	int inserted = 0;
	
	i = length;

	while (i > 0) {		// loop through our path
		if (!strcmp(path[i-1], "left")) {
			if (i == 1) {
				temp->left = node;
				inserted = 1;			// we are done with the (initial, pro heapification) insertion
			} else temp = temp->left;
		} else {
			if (i == 1) {
				temp->right = node;
				inserted = 1;			// we are done with the (initial, pro heapification) insertion
			} else temp = temp->right;
		}
		i--;
	}

	assert(inserted);	// for debugging, we are here, so insertion must have happened
	max_heapify(h, path, length);	// keep the heap's order post-insertion

	h->nodes++;		// we are done with the insertion, so we have one more node in heap

	for (i = 0; i < length; i++) free(path[i]);
	free(path);

	return 0;
}

// to insert in case that we already have node's number in heap
int try_to_fit_to_heap(HeapNode* hn, HeapNode* temp, char*** path, int* length) {
	if (!strcmp(hn->number, temp->number)) {	// we found a heap's node (hn) with the same number as the one that we are trying to insert (temp)
		hn->revenue += temp->revenue;		// basically, keep the revenue, nothing else matters
		clear_heapnode(temp);				// insertion has been concluded, we don't need the temporary heapnode any more
		
		return 1;		// yes, we managed to fit it
	}
	
	if (hn->left != NULL) {
		int retleft = try_to_fit_to_heap(hn->left, temp, path, length);	// try to fit to the left sub-tree
		if (retleft == 1) {
			*path = realloc(*path, ((*length)+1)*sizeof(char*));
			(*path)[*length] = malloc(strlen("left")+1);
			strcpy((*path)[*length], "left");
			(*length)++;
			
			return retleft;	// yes, we managed to fit it
		}
	}
	
	if (hn->right != NULL) {
		int retright = try_to_fit_to_heap(hn->right, temp, path, length);	// try to fit to the right sub-tree
		if (retright == 1) {
			*path = realloc(*path, ((*length)+1)*sizeof(char*));
			(*path)[*length] = malloc(strlen("right")+1);
			strcpy((*path)[*length], "right");
			(*length)++;
			
			return retright;	// yes, we managed to fit it
		}
	}

	return 0;	// no, we couldn't fit it
}

/*
Keep the heap's (max) order post-insertion.
Follow the path's instructions towards the just-inserted node and make the necessary adjustments to the
tree throughout it's course, in order to maintain the tree's heap property.
*/
void max_heapify(Heap* h, char** path, int pathlen) {
	if (!h) return;	// heap has to exist
	
	if (pathlen == 0) return;

	HeapNode *parent = NULL, *kid = h->root;	// the pointers that are going to be used for reference to heapnodes
	double td;
	char* tempstr = NULL;
	int i = pathlen;
	
	while (i > 0) {		// loop through our path
		// set-up our pointers
		parent = kid;
		if (!strcmp(path[i-1], "left")) kid = parent->left;
		else kid = parent->right;

		// check and maintain max-heap property
		if (kid->revenue > parent->revenue) {	// max-heap property has been disturbed
			// we basically want to shift phone-numbers and revenues
			// change numbers
			tempstr = malloc(strlen(kid->number)+1);
			strcpy(tempstr, kid->number);		// keep kid's number
			free(kid->number);
			kid->number = malloc(strlen(parent->number)+1);
			strcpy(kid->number, parent->number);	// new kid's number is current parent's number
			free(parent->number);
			parent->number = malloc(strlen(tempstr)+1);
			strcpy(parent->number, tempstr);		// new parent's number is kid's ex-number
			free(tempstr);
			// change revenues
			td = kid->revenue;	// keeps kid's revenue
			kid->revenue = parent->revenue;	// new kid's revenue is current parent's revenue
			parent->revenue = td;	// new parent's revenue is kid's ex-revenue
		
			kid = h->root;	// fix and reset
			parent = NULL;
			i = pathlen;
		} else i--;
	}
}

int top_k_percent_revenue(Heap* h, double k) {
	double target = (h->total_revenue)*k/100,	// our "targeted" amount of money
			total = 0;
	
	// a supportive structure which keeps pointers to heap's contents in the order in which they would have been printed
	HeapNode** clients = NULL;
	int i, j, count = 0, size = 0;	// for iterating, counting and debugging

	load_heap_to_array(h, &clients, &size);

	// because of the order in which the elements have been exported from the heap, we can use a sorting algorithm
	// as simple as bubble-sort without worrying about high complexity (we will not need too many swaps)
	HeapNode* swap;
	for (i = 0; i < size-1; i++) {
		for (j = 0; j < size-i-1; j++) {
			if (clients[j]->revenue < clients[j+1]->revenue) {	// we want to achieve decreasing order of revenues
				swap = clients[j];
				clients[j] = clients[j+1];
				clients[j+1] = swap;
			}
		}
	}

	for (i = 0; i < size; i++) {	// print our results, regarding our top clients
		if (total < target) {
			printf("number = %s, revenue = %f, which is %f percent of company's total revenue\n", clients[i]->number, clients[i]->revenue, (clients[i]->revenue / h->total_revenue)*100);	// print heapnode's number and revenue-contribution
			total += clients[i]->revenue;
			count++;
		} else break;	// there is no need to go any further
	}

	// free our supporive structure (but don't mess with it's contents)
	free(clients);

	return count;
}

HeapNode* initialize_heapnode(Heap* h, char* number, int duration, int type, int tarrif) {
	HeapNode *hn;
	hn = malloc(sizeof(HeapNode));

	if (!hn) return NULL;

	hn->id = h->nodes+1;

	hn->number = malloc(strlen(number)+1);
	if (!hn->number) return NULL;

	strcpy(hn->number, number);

	hn->revenue = -1;
	int i;
	for (i = 0; i < 5; i++) {
		if ((*(h->charges))[i][0] == type && (*(h->charges))[i][1] == tarrif) {
			if (type == 0) hn->revenue = (*(h->charges))[i][2];
			else hn->revenue = duration * (*(h->charges))[i][2];
		}
	}
	//assert(hn->revenue != -1);	// for debugging, we can't have negative charging after all...

	hn->left = NULL;
	hn->right = NULL;

	return hn;
}

void clear_heapnode(HeapNode* hn) {
	free(hn->number);	// free number's string
	free(hn);		// free heapnode itself
}

void print_heapnode(HeapNode* hn) {
	if (hn == NULL) return;
	printf("id = %d, number = %s, revenue = %f\n", hn->id, hn->number, hn->revenue);
	print_heapnode(hn->left);	// print node's left subtree
	print_heapnode(hn->right);	// print node's right subtree
}

void load_heapnode_to_array(HeapNode* hn, HeapNode*** clients, int* size) {
	if (hn == NULL) return;
	*clients = realloc(*clients, ((*size)+1)*sizeof(HeapNode*));
	(*clients)[*size] = hn;
	(*size)++;
	load_heapnode_to_array(hn->left, clients, size);
	load_heapnode_to_array(hn->right, clients, size);
}