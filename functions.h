#ifndef __FUNCTIONS__
#define __FUNCTIONS__

#include "header.h"

/* Global Variables */
extern int h1NumOfEntries, h2NumOfEntries, bucketSize, connectionSize;

typedef struct hashtable_t HashTable;
typedef struct heap_t Heap;

/* Various Functions */
int myhash(char*, int);	// my hash function
int getConfigurations(char*, double**);
void manage_command(HashTable*, HashTable*, Heap*, char*);
int getOperationsFromFile(char*, HashTable*, HashTable*, Heap*);
int compare_dates(char*, char*);
int compare_times(char*, char*);
int notInContacts(char**, int, char*);
int filter_contacts(char*, char*, HashTable*, HashTable*, char***, int, char***, int, char***);
int isInContacts(char*, int, char**);
int associated(HashTable*, char*, char*);
int find_top_destination(char**, int);
int codeNotIncluded(char*, char**, int);

#endif