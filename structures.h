#ifndef __HASHTABLE__
#define __HASHTABLE__

#include "header.h"

/* Global Variables */
extern int h1NumOfEntries, h2NumOfEntries, bucketSize, connectionSize;

typedef struct record_t {
    char* cdr_uniq_id;
    char* originator_number;
    char* destination_number;
    char* date;
    char* init_time;
    int duration;	// I assume duration is an integer (makes better sense)
    int type; 		// 3 possible types, let: call (1), data (2), SMS (0)
    int tarrif;   	// 3 possible tarrifs, let: hinterland (1), international (call or data) (2), smscost (0)
    int fault_condition;    // call terminated due to: 1: router issue, 2: hand-off, etc
} Record;

typedef struct connection_t {
    Record** record;
    struct connection_t * next;
} Connection;

typedef struct bucketentry_t {
    char* number;
    Connection* p;
} BucketEntry;

typedef struct bucket_t {
    int entries;	// number of entries currently in bucket
    BucketEntry* entry;
    struct bucket_t * next;
} Bucket;

typedef struct hashtable_t {
	char* name;		// hash-table's name (for identification purposes)
    Bucket** bucket;
} HashTable;

/* for Hash-table */
void initialize_hashtable(HashTable*, char*);
void clear_hashtable(HashTable*, char*);
int is_hashtable_empty(HashTable*);
int insert_to_hashtable(HashTable*, Record*);
int delete_from_hashtable(HashTable*, char*, char*);
void print_hashtable(HashTable*, char*);
int find_in_hashtable(HashTable*, char*, char*, char*, char*, char*);
int contacts_in_hashtable(HashTable*, char*, char***, int);
/* for Bucket */
Bucket* initialize_bucket();
void clear_bucket(Bucket*, char*);
int is_bucket_empty(Bucket*);
int insert_to_bucket(Bucket*, char*, Record*);
int delete_from_bucket(Bucket*, char*, char*);
void fix_bucket(Bucket*);			// don't live empty entries in the middle of non empty ones
void print_bucket(Bucket*, FILE*);
int find_in_bucket(Bucket*, char*, char*, char*, char*, char*);
int contacts_in_bucket(Bucket*, char*, char*, char***, int);
/* for BucketEntry */
int is_bucketentry_empty(BucketEntry*);
/* for Connection */
Connection* initialize_connection();
void clear_connection(Connection*, char*);
int is_connection_empty(Connection*);
int insert_to_connection(Connection*, Record*);
int delete_from_connection(Connection*, char*);
void fix_connection(Connection*);	// don't live empty entries in the middle of non empty ones
void print_connection(Connection*, FILE*);
int find_in_connection(Connection*, char*, char*, char*, char*);
int contacts_in_connection(Connection*, char*, char***, int);
/* for Record */
void delete_record(Record*);
void print_record(Record*, FILE*);

#endif