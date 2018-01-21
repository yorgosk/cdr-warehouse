#include "commands.h"

int insert(HashTable* caller_id, HashTable* callee_id, Heap* generatedRevenue, char* command) {
	Record* temp;      // create a cdr-record structure to fill it with what you read
	temp = (Record*)malloc(sizeof(Record));

    if (!temp) return -1;
    
    int pos = 1;
    char *tok;
    tok = strtok(command, " ");	// take "op"-name
    tok = strtok(NULL, ";");
    
    while(tok) {
        // take cdr-record's contents in the right order
    	if (pos == 1) {
    		temp->cdr_uniq_id = malloc(strlen(tok)+1);
    		strcpy(temp->cdr_uniq_id, tok);
    	} else if (pos == 2) {
    		temp->originator_number = malloc(strlen(tok)+1);
            strcpy(temp->originator_number, tok);
    	} else if (pos == 3) {
    		temp->destination_number = malloc(strlen(tok)+1);
            strcpy(temp->destination_number, tok);
    	} else if (pos == 4) {
    		temp->date = malloc(strlen(tok)+1);
    		strcpy(temp->date, tok);
    	} else if (pos == 5) {
    		temp->init_time = malloc(strlen(tok)+1);
    		strcpy(temp->init_time, tok);
    	} else if (pos == 6) {
    		temp->duration = atoi(tok);
    	} else if (pos == 7) {
    		temp->type = atoi(tok);
    	} else if (pos == 8) {
    		temp->tarrif = atoi(tok);
    	} else if (pos == 9) {
    		temp->fault_condition = atoi(tok);
    	} else {
    		return -1;
    	}
    	
    	pos++;
    	tok = strtok(NULL, " \n;");
    }
    /*insert to hashtables*/
    int ret1 = insert_to_hashtable(caller_id, temp);
    if (ret1 == -1) return -1;
    int ret2 = insert_to_hashtable(callee_id, temp);
    if (ret2 == -1) return -1;
    /*insert to heap*/
    if (temp->fault_condition / 100 == 2) { // we don't want to charge calls that never occured
                                            //(if fault-condition isn't of the 2XX kind, this means that there was a fault and we can assume that the communication never happened)
        HeapNode *tnode = initialize_heapnode(generatedRevenue, temp->originator_number, temp->duration, temp->type, temp->tarrif);
        if (!tnode) return -1;  // I count inability to insert to heap as input error (IError) as well
        insert_to_heap(generatedRevenue, tnode);
    }

    return 0;
}

int delete(HashTable* caller_id, char* command) {
	int pos = 1;
    char *tok, *cdr_id = NULL, *caller = NULL;
    tok = strtok(command, " ");	// take "op"-name
    tok = strtok(NULL, " ");
    
    while(tok) {
        // take the deletion command parameters in the right order
    	if (pos == 1) {
            cdr_id = malloc(strlen(tok)+1);
            strcpy(cdr_id, tok);
    	} else if (pos == 2) {
    		caller = malloc(strlen(tok)+1);
    		strcpy(caller, tok);
    	} else {
    		fprintf(stderr, "DError\n");
            free(cdr_id);
            free(caller);
    		return -1;
    	}
    	
    	pos++;
    	tok = strtok(NULL, " \n");
    }

    int ret = delete_from_hashtable(caller_id, cdr_id, caller); // go for the deletion
    if (ret == -1) fprintf(stderr, "DError\n"); // in case you couldn't delete, report it
    
    free(cdr_id);   // free any memory that might have been dynamically allocated
    free(caller);

	return ret;
}

int find(HashTable* caller_id, char* command) {
	// take the "find" command's parameters
    char op[5] = "----", caller[15] = "00000000000000", s1[9] = "--------", s2[9] = "--------", s3[9] = "--------", s4[9] = "--------", *time1 = NULL, *year1 = NULL, *time2 = NULL, *year2 = NULL;
    int scanret = sscanf(command, "%s %s %s %s %s %s", op, caller, s1, s2, s3, s4);
	
    switch (scanret) {
        case 6: // in case we search based in time1, time2, year1, year2 (both time and year parameters)
            if (strlen(caller) != 14 || strlen(s1) != 5 || strlen(s2) != 8 || strlen(s1) != strlen(s3) || strlen(s2) != strlen(s4)) {
                fprintf(stderr, "ERROR! Bad input, try again\n");
                return -1;
            }
           
            time1 = (char*)malloc(strlen(s1)+1);
            strcpy(time1, s1);
            year1 = (char*)malloc(strlen(s2)+1);
            strcpy(year1, s2);
            time2 = (char*)malloc(strlen(s3)+1);
            strcpy(time2, s3);
            year2 = (char*)malloc(strlen(s4)+1);
            strcpy(year2, s4);

            break;
        case 4: // in case we search based in time1, time2 or year1, year2 (just time or just year parameters)
            if (strlen(caller) != 14 || (strlen(s1) != 5 && strlen(s1) != 8) || strlen(s1) != strlen(s2)) {
                fprintf(stderr, "ERROR! Bad input, try again\n");
                return 0;
            }
            
            if (strlen(s1) == 5) {  // based on the HH:MM time-format, if the input was correct, we can find out that we search based on time
                time1 = (char*)malloc(strlen(s1)+1);
                strcpy(time1, s1);
                time2 = (char*)malloc(strlen(s2)+1);
                strcpy(time2, s2);
            } else {    // "year" has DDMMYYYY format, we search based on year
                year1 = (char*)malloc(strlen(s1)+1);
                strcpy(year1, s1);
                year2 = (char*)malloc(strlen(s2)+1);
                strcpy(year2, s2);
            }

            break;
        case 2: // we search based just on phone number
            if (strlen(caller) != 14) {
                fprintf(stderr, "ERROR! Bad input, try again\n");
                return 0;
            }

            break;
        default:
            fprintf(stderr, "ERROR! Bad input, try again\n");
            return 0;
    }

	int ret = find_in_hashtable(caller_id, caller, time1, year1, time2, year2);    // do the "finding"

    // free any memory that might have been dynamically allocated
	if (time1) {
		free(time1);
		free(time2);
	}
	if (year1) {
		free(year1);
		free(year2);
	}

	return ret;
}

int lookup(HashTable* callee_id, char* command) {
	// take the "lookup" command's parameters
    char op[7] = "------", callee[15] = "00000000000000", s1[9] = "--------", s2[9] = "--------", s3[9] = "--------", s4[9] = "--------", *time1 = NULL, *year1 = NULL, *time2 = NULL, *year2 = NULL;
    int scanret = sscanf(command, "%s %s %s %s %s %s", op, callee, s1, s2, s3, s4);
    
    switch (scanret) {
        case 6: // in case we search based in time1, time2, year1, year2 (both time and year parameters)
            if (strlen(callee) != 14 || strlen(s1) != 5 || strlen(s2) != 8 || strlen(s1) != strlen(s3) || strlen(s2) != strlen(s4)) {
                fprintf(stderr, "ERROR! Bad input, try again\n");
                return -1;
            }
            
            time1 = (char*)malloc(strlen(s1)+1);
            strcpy(time1, s1);
            year1 = (char*)malloc(strlen(s2)+1);
            strcpy(year1, s2);
            time2 = (char*)malloc(strlen(s3)+1);
            strcpy(time2, s3);
            year2 = (char*)malloc(strlen(s4)+1);
            strcpy(year2, s4);

            break;
        case 4: // in case we search based in time1, time2 or year1, year2 (just time or just year parameters)
            if (strlen(callee) != 14 || (strlen(s1) != 5 && strlen(s1) != 8) || strlen(s1) != strlen(s2)) {
                fprintf(stderr, "ERROR! Bad input, try again\n");
                return -1;
            }
            
            if (strlen(s1) == 5) {  // based on the HH:MM time-format, if the input was correct, we can find out that we search based on time
                time1 = (char*)malloc(strlen(s1)+1);
                strcpy(time1, s1);
                time2 = (char*)malloc(strlen(s2)+1);
                strcpy(time2, s2);
            } else {    // "year" has DDMMYYYY format, we search based on year
                year1 = (char*)malloc(strlen(s1)+1);
                strcpy(year1, s1);
                year2 = (char*)malloc(strlen(s2)+1);
                strcpy(year2, s2);
            }

            break;
        case 2: // we search based just on phone number
            if (strlen(callee) != 14) {
                fprintf(stderr, "ERROR! Bad input, try again\n");
                return -1;
            }

            break;
        default:
            fprintf(stderr, "ERROR! Bad input, try again\n");
            return -1;
    }

	int ret = find_in_hashtable(callee_id, callee, time1, year1, time2, year2);    // do the "looking-up", essentially find something in the calle hashtable

    // free any memory that might have been dynamically allocated
	if (time1) {
		free(time1);
		free(time2);
	}
	if (year1) {
		free(year1);
		free(year2);
	}

	return ret;
}

int indist(HashTable* caller_id, HashTable* callee_id, char* command) {
	int pos = 1;
    char *tok, *caller1 = NULL, *caller2 = NULL;
    tok = strtok(command, " ");	// take "op"-name
    tok = strtok(NULL, " ");
    
    while(tok) {
        // take "indist" command's parameters in the right order
    	if (pos == 1) {
    		caller1 = (char*)malloc(strlen(tok)+1);
            strcpy(caller1, tok);
    	} else if (pos == 2) {
    		caller2 = (char*)malloc(strlen(tok)+1);
            strcpy(caller2, tok);
    	} else {
    		fprintf(stderr, "ERROR! Bad input, try again.\n");
            free(caller1);
            free(caller2);
    		return -1;
    	}
    	
    	pos++;
    	tok = strtok(NULL, " \n");
    }
    
    char **contacts1 = NULL, **contacts2 = NULL, **finalcontacts = NULL;	// who has contacted caller1, who caller2 and final contacts respectively
    int c11 = contacts_in_hashtable(caller_id, caller1, &contacts1, 0);     // who has "caller1" called
    int c12 = contacts_in_hashtable(callee_id, caller1, &contacts1, c11);   // who has called "caller1"
    int c21 = contacts_in_hashtable(caller_id, caller2, &contacts2, 0);     // who has "caller2" called
    int c22 = contacts_in_hashtable(callee_id, caller2, &contacts2, c21);   // who has called "caller2"

    // construct final contacts
    int fc = filter_contacts(caller1, caller2, caller_id, callee_id, &contacts1, c12, &contacts2, c22, &finalcontacts);
    // print our results
    int i;
    printf("Contacts indist %s and %s:\n", caller1, caller2);
    for (i = 0; i < fc; i++) printf("\t%s\n", finalcontacts[i]);
    // free any memory that might have been dynamically allocated
    free(caller1);
    free(caller2);
    for (i = 0; i < c12; i++) free(contacts1[i]);
    free(contacts1);
    for (i = 0; i < c22; i++) free(contacts2[i]);
    free(contacts2);
    for (i = 0; i < fc; i++) free(finalcontacts[i]);
    free(finalcontacts);

	return fc;
}

int topdest(HashTable* caller_id, char* command) {
	int pos = 1;
    char *tok, *caller = NULL;
    tok = strtok(command, " ");	// take "op"-name
    tok = strtok(NULL, " \n");
    
    while(tok) {
        // take "topdest" command's arguments in the right order
    	if (pos == 1) {
    		caller = (char*)malloc(strlen(tok)+1);
            strcpy(caller, tok);
    	} else {
    		fprintf(stderr, "ERROR! Bad input, try again.\n");
            free(caller);
    		return -1;
    	}
    	
    	pos++;
    	tok = strtok(NULL, " \n");
    }

    char **contacts = NULL; // everybody that "caller" has called and the top destination
    int c = contacts_in_hashtable(caller_id, caller, &contacts, 0);
    int ret = find_top_destination(contacts, c);    // find the top destination(s)
    // free any memory that might have been dynamically allocated
    int i;
    for (i = 0; i < c; i++) free(contacts[i]);
    free(contacts);
    free(caller);

	return ret;
}

int top(Heap* generatedRevenue, char* command) {
	int pos = 1;
    double k;   // I operate under the assumption that for "top-k%" k can be a float number
    char *tok;
    tok = strtok(command, " ");	// take "op"-name
    tok = strtok(NULL, " ");
    
    while(tok) {
        // take "top" command's arguments in the right order
    	if (pos == 1) {
    		k = atoi(tok);
    	} else {
    		return 0;
    	}
    	
    	pos++;
    	tok = strtok(NULL, " \n");
    }
    // find the clients that have generated the top k-percent of the company's total revenue
    int ret = top_k_percent_revenue(generatedRevenue, k);

	return ret;
}

int bye(HashTable* caller_id, HashTable* callee_id, Heap* generatedRevenue) {
	// clear existing structures
	clear_hashtable(caller_id, "");
    clear_hashtable(callee_id, "-cdr");
    /*CLEAR HEAP*/
    double ***temp = take_charges(generatedRevenue);    // so that we don't lose them
    clear_heap(generatedRevenue);
    
    // initialize new structures
    initialize_hashtable(caller_id, "caller");
    initialize_hashtable(callee_id, "callee");
    /*INITIALIZE HEAP*/
    initialize_heap(generatedRevenue, temp);

	return 0;
}

int print(HashTable* caller_id, HashTable* callee_id, char* command) {
	int pos = 1;
	HashTable* ht;
    char *tok, *htName = NULL;
    tok = strtok(command, " ");	// take "op"-name
    tok = strtok(NULL, " \n");
    
    while(tok) {
        // take the "print" command parameters in the right order
    	if (pos == 1) {
    		htName = (char*)malloc(strlen(tok)+1);
    		strcpy(htName, tok);
    	} else {
    		fprintf(stderr, "ERROR! Bad input, try again.\n");
    		if (htName) free(htName);
    		return -1;
    	}
    	
    	pos++;
    	tok = strtok(NULL, " \n");
    }
    
    if (!strcmp(htName, "caller") || !strcmp(htName, "hashtable1")) ht = caller_id; // in case we print the caller_id hashtable
    else if (!strcmp(htName, "callee") || !strcmp(htName, "hashtable2")) ht = callee_id;    // in case we print the callee_id hashtable
    else {
    	fprintf(stderr, "ERROR! Bad input, try again.\n");
    	if (htName) free(htName);
    	return -1;
    }

    print_hashtable(ht, "");    // print the requested hashtable (on screen)

    if (htName) free(htName);

	return 0;
}

int dump(HashTable* caller_id, HashTable* callee_id, char* command) {
	int pos = 1;
	HashTable* ht;
    char *tok, *htName = NULL, *filename = NULL;
    tok = strtok(command, " ");	// take "op"-name
    tok = strtok(NULL, " ");
    
    while(tok) {
        // take the "dump" command parameters in the right order
    	if (pos == 1) {
    		htName = (char*)malloc(strlen(tok)+1);
    		strcpy(htName, tok);
    	} else if (pos == 2) {
    		filename = (char*)malloc(strlen(tok)+1);
    		strcpy(filename, tok);
    	} else {
    		fprintf(stderr, "ERROR! Bad input, try again.\n");
    		if (htName) free(htName);
    		if (filename) free(filename);
    		return -1;
    	}
    	
    	pos++;
    	tok = strtok(NULL, " \n");
    }
    
    if (!strcmp(htName, "caller") || !strcmp(htName, "hashtable1")) ht = caller_id; // in case we "dump" the caller_id hashtable
    else if (!strcmp(htName, "callee") || !strcmp(htName, "hashtable2")) ht = callee_id;    // in case we "dump" the callee_id hashtable
    else {
    	fprintf(stderr, "ERROR! Bad input, try again.\n");
    	if (htName) free(htName);
    	if (filename) free(filename);
    	return -1;
    }
   
    print_hashtable(ht, filename);  // "dump" the requested hashtable, essentially print it, but in a file
    
    if (htName) free(htName);
    if (filename) free(filename);

	return 0;
}