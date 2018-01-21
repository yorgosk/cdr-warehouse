#ifndef __COMMANDS__
#define __COMMANDS__

#include "header.h"

/* Global Variables */
extern int h1NumOfEntries, h2NumOfEntries, bucketSize, connectionSize;

typedef struct hashtable_t HashTable;
typedef struct heap_t Heap;

/* COMMANDS' FUNCTIONS */
int insert(HashTable*, HashTable*, Heap*, char*);
int delete(HashTable*, char*);
int find(HashTable*, char*);
int lookup(HashTable*, char*);
int indist(HashTable*, HashTable*, char*);
int topdest(HashTable*, char*);
int top(Heap*, char*);
int bye(HashTable*, HashTable*, Heap*);
int print(HashTable*, HashTable*, char*);
int dump(HashTable*, HashTable*, char*);

#endif