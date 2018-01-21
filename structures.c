#include "structures.h"

/* for Hash-table */
void initialize_hashtable(HashTable* ht, char* person) {
	int size, i;
	
	ht->name = (char*)malloc(strlen(person)+1);	// name the hashtable, so we know if it's the callers' or the callees' one
	strcpy(ht->name, person);
	
	if (!strcmp(ht->name, "caller")) size = h1NumOfEntries;	// pick the proper size for our hashtable
	else size = h2NumOfEntries;
	
	ht->bucket = (Bucket**)malloc(size*sizeof(Bucket*));	// create bucket-pointers array

	for (i = 0; i < size; i++) ht->bucket[i] = NULL;		// initialize bucket-pointers array
}

void clear_hashtable(HashTable* ht, char* selector) {
	int size, i;
	
	if (!strcmp(ht->name, "caller")) size = h1NumOfEntries;	// pick the proper size for our hashtable
	else size = h2NumOfEntries;
	
	for (i = 0; i < size; i++) {	// clear each of the hashtable buckets
		clear_bucket(ht->bucket[i], selector);
		ht->bucket[i] = NULL;
	}
	
	free(ht->bucket);				// free buckets' array
	ht->bucket = NULL;
	free(ht->name);					// free hashtable's name
}

int is_hashtable_empty(HashTable* ht) {	// a simple check, just in case it's ever needed
	int i, size;

	if (!strcmp(ht->name, "caller")) size = h1NumOfEntries;	// pick the proper size for our hashtable
	else size = h2NumOfEntries;

	for (i = 0; i < size; i++)
		if (ht->bucket[i] != NULL)	// if there is at least one bucket
			return 0;	// negative, not empty

	return 1;	// positive, empty
}

int insert_to_hashtable(HashTable* ht, Record* cdrp) {
	int hash, ret;
	
	if (!strcmp(ht->name, "caller")) {	// hash with the proper number (/hashtable size) based on hashtable's name
		hash = myhash(cdrp->originator_number, h1NumOfEntries);
	} else {
		hash = myhash(cdrp->destination_number, h2NumOfEntries);
	}

	if (ht->bucket[hash] == NULL) {	// if there is no bucket in the hashtable position
		ht->bucket[hash] = initialize_bucket();	// initialize a bucket in this position
	}
	if(ht->bucket[hash] == NULL) return -1;	// we had an "IError"
	
	ret = insert_to_bucket(ht->bucket[hash], ht->name, cdrp);	// insert cdr-record to bucket

	return ret;
}

int delete_from_hashtable(HashTable* ht, char* cdr_id, char* number) {
	//assert(!strcmp(ht->name, "caller"));	// for debugging
	int ret = 0;
	int hash;

	hash = myhash(number, h1NumOfEntries);	// find the bucket from where we are deleting
	
	if (ht->bucket[hash] == NULL)	// if there is no such bucket
		ret = -1;	// number not found
	else {
		ret = delete_from_bucket(ht->bucket[hash], cdr_id, number);
		if (ret != -1) fix_bucket(ht->bucket[hash]);	// if deletion indeed happened, there may be a need for bucket-list fixing
	}

	if (is_bucket_empty(ht->bucket[hash])) {
		clear_bucket(ht->bucket[hash], "");	// we want to keep the cdr-info in memory
		ht->bucket[hash] = NULL;
	}

	return ret;
}

void print_hashtable(HashTable* ht, char* filename) {
	int i, size;
	FILE *fp = stdout;

	if (strcmp(filename, "")) {
		fp = fopen(filename, "a");

		if (!fp) {
			fprintf(stderr, "ERROR! Can't open file named %s\n", filename);
			return;
		}
	}

	if (!strcmp(ht->name, "caller")) size = h1NumOfEntries;
	else size = h2NumOfEntries;
		
	fprintf(fp, "Printing hashtable named: %s\n", ht->name);
	// print each existing bucket
	for (i = 0; i < size; i++) {
		if (ht->bucket[i]) {	// print each hashtable's bucket
			print_bucket(ht->bucket[i], fp);
		}
	}

	if (strcmp(filename, "")) fclose(fp);
}

int find_in_hashtable(HashTable* ht, char* number, char* time1, char* year1, char* time2, char* year2) {
	int hash, ret = 0;
	
	if (!strcmp(ht->name, "caller")) {	// hash with the proper number (/hashtable size) based on hashtable's name
		hash = myhash(number, h1NumOfEntries);
	} else {
		hash = myhash(number, h2NumOfEntries);
	}

	// if there is such a bucket, call it's "find"-function
	if (ht->bucket[hash] != NULL) ret = find_in_bucket(ht->bucket[hash], number, time1, year1, time2, year2);

	return ret;
}

int contacts_in_hashtable(HashTable* ht, char* number, char*** contactsp, int num) {
	int hash, ret = num;	// if we can't find a position in the hashtable, we still want to return (just) num
	if (!strcmp(ht->name, "caller")) {	// hash with the proper number (/hashtable size) based on hashtable's name
		hash = myhash(number, h1NumOfEntries);
	} else {
		hash = myhash(number, h2NumOfEntries);
	}
	// if there is such a bucket, call it's "contacts"-function
	if (ht->bucket[hash] != NULL) ret = contacts_in_bucket(ht->bucket[hash], ht->name, number, contactsp, num);

	return ret;
}

/* for Bucket */
Bucket* initialize_bucket() {
	Bucket* b;
	b = malloc(bucketSize);	// allocate space for the bucket

	if (!b) return NULL;	// in case we couldn't allocate memory

	b->entries = 0;	// we it need to track how many entries we have in bucket
	b->entry = malloc(((bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry))*sizeof(BucketEntry));	// our bucket-entries array
	
	if (!b->entry) return NULL;		// in case we couldn't allocate memory

	b->next = NULL;
	
	int i, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry);
	for (i = 0; i < size; i++) {	// initialize entries' members
		b->entry[i].number = NULL;
		b->entry[i].p = NULL;
	}

	return b;	// return a pointer to our just-created bucket
}

// clear the whole bucket-list
void clear_bucket(Bucket* b, char* selector) {
	int i, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry);
	
	while(b != NULL) {	// for each bucket of the bucket-list
		for(i = 0; i < size; i++) {	// for each bucket's entry
			free(b->entry[i].number);
			b->entry[i].number = NULL;
			clear_connection(b->entry[i].p, selector);	// clear the connection
			b->entry[i].p = NULL;
		}
		free(b->entry);	// clear bucket-entries' array
		b->entry = NULL;
		
		Bucket* prev = b;	// keep the memory-address of our current (previous-to-be) bucket
		b = b->next;	// go to the next (now-current) bucket of the list (which is an actual bucket or NULL (we were in the last bucket))
		free(prev);	// free the previous (ex-current) bucket
	}
}

int is_bucket_empty(Bucket* b) {
	while (b) {
		if(b->entries)	// if there are entries in bucket
			return 0;	// negative, not empty

		b = b->next;
	}

	return 1;	// positive, empty
}

int insert_to_bucket(Bucket* b, char* person, Record* cdrp) {
	int i = 0, ret, inserted = 0, end = 0, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry);
	
	// try to insert it in the existing buckets
	do {	// go through the bucket list
		for (i = 0; i < size; i++) {	// go through the current bucket
			// if there is already an entry for the right number in this bucket
			if (b->entry[i].number != NULL && ((!strcmp(person, "caller") && !strcmp(b->entry[i].number, cdrp->originator_number)) || (!strcmp(person, "callee") && !strcmp(b->entry[i].number, cdrp->destination_number)))) {
				if (b->entry[i].p == NULL) {
					b->entry[i].p = initialize_connection();	// if there is a need of a connections' structure, create one
				
					if (!b->entry[i].p) return -1;
				}
				
				ret = insert_to_connection(b->entry[i].p, cdrp);	// insert entry to connections' structure
				inserted = 1;	// insertion concluded
				break;
			}
		}

		if (inserted)
			break;
		else if ((b->entries == size) && b->next != NULL)
			b = b->next;	// if insertion wasn't concluded and we are in a full bucket, go to the next one (if you can)
		else if (b->entries != size) {	// if there is space in the current bucket
			// go to the first empty place in the bucket
			for (i = 0; i < size; i++) if (b->entry[i].number == NULL) break;

			if (!strcmp(person, "caller")) {
				b->entry[i].number = malloc(strlen(cdrp->originator_number)+1);
				strcpy(b->entry[i].number, cdrp->originator_number);	// insert the correct number right where we stopped the above iteration (that's why I have initialized i = 0 outside of "for")
			} else {
				b->entry[i].number = malloc(strlen(cdrp->destination_number)+1);
				strcpy(b->entry[i].number, cdrp->destination_number);
			}
			
			b->entry[i].p = initialize_connection();	// initialize a connections' structure to point to
			if (!b->entry[i].p) return -1;
			
			ret = insert_to_connection(b->entry[i].p, cdrp);	// insert to connections' structure
			if (ret != -1) b->entries++;		// if insertion indeed happened we have one more entry
			inserted = 1;	// insertion concluded
			break;
		} else
			end = 1;	// else end this loop
	} while (!end);
	
	// we couldn't insert it to any of the existing buckets (and now we are in the last bucket of the bucket-list)
	if (!inserted) {	// if insertion hasn't been concluded yet
		b->next = initialize_bucket();	// create a next bucket
		b = b->next;	// go to the next bucket
		
		// insert it at the beginning of the next bucket
		if (!strcmp(person, "caller")) {
			b->entry[0].number = malloc(strlen(cdrp->originator_number)+1);
			strcpy(b->entry[0].number, cdrp->originator_number);	// insert the correct number in the new-bucket's first position
		} else {
			b->entry[0].number = malloc(strlen(cdrp->destination_number)+1);
			strcpy(b->entry[0].number, cdrp->destination_number);
		}
		
		b->entry[0].p = initialize_connection();	// initialize a connections' structure to point to

		if (!b->entry[0].p) return -1;

		ret = insert_to_connection(b->entry[0].p, cdrp);	// insert to connections' structure
		b->entries++;		// we have one more entry
	}

	return ret;
}

int delete_from_bucket(Bucket* b, char* cdr_id, char* number) {
	int i, found = 0, ret = -1, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry);
	
	do {
		for (i = 0; i < size; i++) {
			if (!strcmp(b->entry[i].number, number) && b->entry[i].p != NULL) {
				ret = delete_from_connection(b->entry[i].p, cdr_id);
				found = 1;

				if (ret != -1) fix_connection(b->entry[i].p);	// if deletion indeed happened, there may be a need for fixing

				if(is_connection_empty(b->entry[i].p)) {
					clear_connection(b->entry[i].p, "");
					free(b->entry[i].number);
					b->entry[i].number = NULL;
					b->entry[i].p = NULL;
					b->entries--;	// we have one less entry
				}
				break;
			}
		}
		b = b->next;
	} while (!found && b);
	
	return ret;
}

/* Fix bucket-list.
If there is an empty position in the middle of the bucket-list, find the last entry of the whole bucket list,
take it's contents and put them in the empty position. It's rational, that if we fix all the time, each time,
there will be at most one position needed to be fixed.
*/
void fix_bucket(Bucket* b) {
	int i, j, fixed = 0, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry);
	Bucket *temp = b, *prev = NULL;	// keep "b"'s initial value for future iteration(s)
	char *tempnumber = NULL;
	Connection *tempp = NULL;

	do {
		for (i = 0; i < size; i++) {
			// if there is need for fixing
			if ((i < size-1 && is_bucketentry_empty(&(b->entry[i])) && !is_bucketentry_empty(&(b->entry[i+1]))) || (i == size-1 && is_bucketentry_empty(&(b->entry[i])) && b->next != NULL)) {
				// find bucket-list's last entry
				// go to the last bucket
				while(temp->next) {
					prev = temp;		// always keep previous bucket's address
					temp = temp->next;
				}
				// find it's last entry
				for (j = 0; j < size; j++) {
					if ((j < size-1 && !is_bucketentry_empty(&(temp->entry[j])) && is_bucketentry_empty(&(temp->entry[j+1]))) || (j == size-1 && !is_bucketentry_empty(&(temp->entry[j])))) {
						// keep entry's contents
						tempnumber = malloc(strlen(temp->entry[j].number)+1);
						strcpy(tempnumber, temp->entry[j].number);
						tempp = temp->entry[j].p;
						// clear entry
						free(temp->entry[j].number);
						temp->entry[j].number = NULL;
						temp->entry[j].p = NULL;
						break;	// there's no need to loop any further
					}
				}
				//assert(tempnumber != NULL && tempp != NULL);
				// use last entry to fill the gap
				b->entry[i].number = malloc(strlen(tempnumber)+1);
				strcpy(b->entry[i].number, tempnumber);
				b->entry[i].p = tempp;

				fixed = 1;
				// if ex-last entry's bucket (last bucket) is now empty then we don't need it anymore
				if (is_bucket_empty(temp)) {
					clear_bucket(temp, "");	// we want to keep the cdr-info in memory
					prev->next = NULL;
				}

				free(tempnumber);
			}

			if(fixed) break;
		}

		b = b->next;	// go to the next bucket
	} while (!fixed && b);
}

void print_bucket(Bucket* b, FILE* fp) {
	fprintf(fp, "\tPrinting a bucket list:\n");
	
	int c = 1, i, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry);
	do {	// print bucket list
		fprintf(fp, "\t\tPrinting bucket no.%d\n", c);
		for (i = 0; i < size; i++) {
			if(b->entry[i].p == NULL) continue;	// when we reach an empty position we have reached the end of the bucket
			
			fprintf(fp, "\t\t\tFor number %s we have:\n", b->entry[i].number);
			print_connection(b->entry[i].p, fp);
		}
		fprintf(fp, "\n");
		c++;
		b = b->next;
	} while (b);
}

int find_in_bucket(Bucket* b, char* number, char* time1, char* year1, char* time2, char* year2) {
	int i, found = 0, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry), ret = 0;
	
	do {	// go through the bucket list
		for (i = 0; i < size; i++) {
			if (!is_bucketentry_empty(&(b->entry[i])) && !strcmp(b->entry[i].number, number)) {
				if (b->entry[i].p != NULL)
					ret = find_in_connection(b->entry[i].p, time1, year1, time2, year2);
				break;
			}
		}
		if (found) break;
		b = b->next;
	} while(b);

	return ret;
}

int contacts_in_bucket(Bucket* b, char* htName, char* number, char*** contactsp, int num) {
	int i, found = 0, ret = num, size = (bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(BucketEntry);
	
	do {	// go through the bucket list
		for (i = 0; i < size; i++) {
			if (b->entry[i].number && !strcmp(b->entry[i].number, number)) {
				if (b->entry[i].p != NULL)
					ret = contacts_in_connection(b->entry[i].p, htName, contactsp, num);
				
				found = 1;
				break;
			}
		}
		if (found) break;
		b = b->next;
	} while(b);

	return ret;
}

/* for BucketEntry 
1, positive, is empty
0, negative, is not empty
*/
int is_bucketentry_empty(BucketEntry *bep) {
	return (bep->number == NULL && bep->p == NULL);
}

/* for Connection */
Connection* initialize_connection() {
	Connection* c;
	c = malloc(connectionSize);	// allocate space for the connection

	if (!c) return NULL;

	c->record = malloc(connectionSize-sizeof(Connection*));	// our record-pointers array

	if (!c->record) return NULL;
	
	int i, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);
	for (i = 0; i < size; i++) c->record[i] = NULL;	// initialize all of the array's pointers to point nowhere (NULL)
	c->next = NULL;

	return c;	// return a pointer to our just-created connection
}

void clear_connection(Connection* c, char* selector) {
	int i, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);
	
	while (c != NULL) {	// for each connection of the connection-list
		for (i = 0; i < size; i++) {	// for each connections' structure entry
			if (!strcmp(selector, "-cdr") && c->record[i] != NULL) {	// if we are explicitly commanded to clear the cdr-record where the entry points to as well, then do it
				delete_record(c->record[i]);	// delete cdr-record
			}
			c->record[i] = NULL;
		}
		free(c->record);	// clear record-pointers' array
		c->record = NULL;
		Connection* prev = c;	// keep the memory-address of our current (previous-to-be) connections' struct
		c = c->next;	// go to the next (now-current) connections' struct of the list (which is an actual connections' struct or NULL (we were in the last connections' struct))
		free(prev);	// free the previous (ex-current) connections' struct
	}
}

int is_connection_empty(Connection* conn) {
	int i, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);

	while (conn) {
		for (i = 0; i < size; i++)
			if (conn->record[i] != NULL)
				return 0;	// negative, not empty

		conn = conn->next;
	}

	return 1;	// positive, empty
}

int insert_to_connection(Connection* conn, Record* cdrp) {
	int i, ret = 0, inserted = 0, end = 0, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);
	
	do {	// go through the connections' structure list
		for (i = 0; i < size; i++) {	// go through all the entries of the current connections' structure
			if (conn->record[i] == NULL) {
				conn->record[i] = cdrp;	// if this record-pointer points nowhere, then set it to point to our cdr-record
				inserted = 1;	// insertion concluded
				break;	// exit this loop
			}
		}
		if (inserted) break;
		else if (conn->next != NULL) conn = conn->next;	// go to the next connections' structure if you can
		else end = 1;
	} while (!end);
	
	if (!inserted) {	// if insertion hasn't been concluded yet
		conn->next = initialize_connection();	// create a next connections' structure

		if (!conn->next) return -1;

		conn = conn->next;	// go to the next connections' structure
		conn->record[0] = cdrp;	// set it's first entry-pointer to point to our cdr-record
	}

	return ret;
}

int delete_from_connection(Connection* conn, char* cdr_id) {
	int i, found = 0, ret = -1, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);

	do {	// go through the connections' structure list
		for (i = 0; i < size; i++) {	// go through all the entries of the current connections' structure
			if (conn->record[i] != NULL && !strcmp(cdr_id, conn->record[i]->cdr_uniq_id)) {	// if we found the number that we want to "delete"
				conn->record[i] = NULL;
				found = 1;
				ret = 0;
				break;
			}
		}
		conn = conn->next;
	} while (!found && conn);
	
	return ret;
}

/* Fix connection-structs-list.
If there is an empty position in the middle of the connection-structs-list, find the last entry of the whole list,
take it's content and put it in the empty position. It's rational, that if we fix all the time, each time,
there will be at most one position needed to be fixed.
*/
void fix_connection(Connection* conn) {
	int i, j, fixed = 0, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);
	Connection *temp = conn, *prev = NULL;	// keep "conn"'s initial value for future iteration(s)
	Record *temprecord = NULL;

	do {
		for (i = 0; i < size; i++) {
			// if there is need for fixing
			if ((i < size-1 && conn->record[i] == NULL && conn->record[i+1] != NULL) || (i == size-1 && conn->record[i] == NULL && conn->next != NULL)) {
				// find connection-structs-list's last entry
				// go to the last connection-struct
				while(temp->next) {
					prev = temp;		// always keep previous connection-structs's address
					temp = temp->next;
				}
				// find it's last entry
				for (j = 0; j < size; j++) {
					if ((j < size-1 && temp->record[j] != NULL && temp->record[j+1] == NULL) || (j == size-1 && temp->record[j] != NULL)) {
						// keep entry's contents
						temprecord = temp->record[j];
						// clear entry
						temp->record[j] = NULL;
						break;	// there's no need to loop any further
					}
				}
				// use last entry to fill the gap
				conn->record[i] = temprecord;

				fixed = 1;
				// if ex-last entry's connection-structure (last connection-struct) is now empty then we don't need it anymore
				if (is_connection_empty(temp)) {
					clear_connection(temp, "");	// we want to keep the cdr-info in memory
					prev->next = NULL;
				}
			}

			if(fixed) break;
		}

		conn = conn->next;	// go to the next connection-struct
	} while (!fixed && conn);
}

void print_connection(Connection* conn, FILE* fp) {
	fprintf(fp, "\t\t\t\tPrinting a connections' structure list\n");

	int c = 1, i, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);
	
	do {	// print bucket list
		fprintf(fp, "\t\t\t\t\tPrinting connections' structure no.%d\n", c);
		for (i = 0; i < size; i++) {
			if (conn->record[i] != NULL)
				print_record(conn->record[i], fp);
		}
		c++;
		conn = conn->next;
	} while (conn);
}

int find_in_connection(Connection* conn, char* time1, char* year1, char* time2, char* year2) {
	int i, size = (connectionSize-sizeof(Connection*))/sizeof(Record*), count = 0;
	
	do {	// go through the connections list
		for (i = 0; i < size; i++) {
			if (conn->record[i] != NULL) {
				if (year1) {	// if we are filtering with year
					if (compare_dates(conn->record[i]->date, year1) <= 0 && compare_dates(conn->record[i]->date, year2) >= 0) {
						if (time1) {	// if we are filtering with time as well
							if (compare_times(conn->record[i]->init_time, time1) <= 0 && compare_times(conn->record[i]->init_time, time2) >= 0) {
								print_record(conn->record[i], stdout);
								count++;
							}
						} else {
							print_record(conn->record[i], stdout);
							count++;
						}
					}
				} else if (time1) {	// if we are filtering just with time
					if (compare_times(conn->record[i]->init_time, time1) <= 0 && compare_times(conn->record[i]->init_time, time2) >= 0) {
						print_record(conn->record[i], stdout);
						count++;
					}
				} else	// if we just want a number, without any filtering
					print_record(conn->record[i], stdout);
					count++;
			}
		}
		conn = conn->next;
	} while(conn);

	return count;	// return how many items where found
}

int contacts_in_connection(Connection* conn, char* htName, char*** contactsp, int num) {
	int i, count = num, size = (connectionSize-sizeof(Connection*))/sizeof(Record*);
	
	do {	// go through the connections list
		for (i = 0; i < size; i++) {
			if (conn->record[i] != NULL) {
				if (!strcmp(htName, "caller")) {
					if (!isInContacts(conn->record[i]->destination_number, count, *contactsp)) {
						*contactsp = realloc(*contactsp, (count+1)*sizeof(char*));
						(*contactsp)[count] = malloc(strlen(conn->record[i]->destination_number)+1);
						strcpy((*contactsp)[count], conn->record[i]->destination_number);

						count++;
					}
				} else {	// if "callee"
					if (!isInContacts(conn->record[i]->originator_number, count, *contactsp)) {
						*contactsp = realloc(*contactsp, (count+1)*sizeof(char*));
						(*contactsp)[count] = malloc(strlen(conn->record[i]->originator_number)+1);
						strcpy((*contactsp)[count], conn->record[i]->originator_number);
						count++;
					}
				}
			}
		}
		conn = conn->next;
	} while(conn);

	return count;
}

/* for Record */
void delete_record(Record* r) {
	free(r->cdr_uniq_id);	// delete each record's field that required dynamic memory allocation
	free(r->originator_number);
	free(r->destination_number);
    free(r->date);
    free(r->init_time);
    free(r);	// then delete the record itself
}

void print_record(Record* r, FILE* fp) {	// print the record using the format in which you read it
	fprintf(fp, "\t\t\t\t\t\t%s;%s;%s;%s;%s;%d;%d;%d;%d\n",
		r->cdr_uniq_id, r->originator_number, r->destination_number, r->date, r->init_time, r->duration, r->type, r->tarrif, r->fault_condition);
}