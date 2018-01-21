#include "functions.h"

/*
My hash function.
Takes as input a number of the ABC-DEFGHIJKLM format and the size of the hashtable for which I hash.
It returns the place of the number in the hashtable.
*/
int myhash(char* number, int htSize) {
	int i, sum = 0;
	for (i = 0; i < strlen(number); i++)	// go through each 'char'-typed digit of the number
		sum += number[i];					// and sum their ASCII codes

	return sum % htSize;	// modulo, in range of [0, htSize-1]
}

int getConfigurations(char* configFile, double** config) {
	int error = 0;
	FILE *fp;
	fp = fopen(configFile, "r");	// (try to) open the configuration file
	if (fp == NULL) {	// in case it couldn't open it
		fprintf(stderr, "ERROR! Unable to open configuration file named %s\n", configFile);
		error = -1;
		return error;
	}
	
	char* configuration = NULL;   // forces getline to allocate with malloc
    size_t len = 0; // ignored when line=NULL
    ssize_t read;
    int i = 0, j;
    while ((read = getline(&configuration, &len, fp)) != -1 && i < 5) {
        if(configuration[0] == '#') {	// if you just read a comment
        	free(configuration);
        	configuration = NULL;   // forces getline to allocate with malloc
        	continue;	// go to read next line
        }

        char* temp = NULL;      // temporary string, so that I don't lose the initial configuration while tokenizing
        temp = malloc(strlen(configuration)+1);
        strcpy(temp, configuration);
        
        j = 0;
	    char* tok;
	    tok = strtok(temp, ";");
	    while(tok && j < 3) {
	    	config[i][j] = atof(tok);
	    	
	    	j++;
	    	tok = strtok(NULL, "\n;");
	    }
        free(temp); // free memory allocated for temporary string
        i++;
    }
    free(configuration);  // free memory allocated by getline

    fclose(fp);

	return error;
}

void manage_command(HashTable* caller_id, HashTable* callee_id, Heap* generatedRevenue, char* command) {
	int ret;
	char* temp = NULL;      // temporary string, so that I don't lose the initial command while tokenizing
    temp = (char*)malloc(strlen(command)+1);
    strcpy(temp, command);
    if (!strcmp(command, "\n")) {
        free(temp);
        return;   // if just "enter" was pressed, return so we can loop again (in main)
    }
    
    // for the tokenization I consulted: http://stackoverflow.com/questions/4513316/split-string-in-c-every-white-space
    char* op;
    op = strtok(temp, " \n");   // '\n' for bye which is followed by an immediate 'enter'
    if(!strcmp(op, "insert")) {
        ret = insert(caller_id, callee_id, generatedRevenue, command);
        
        if (!ret) printf("OK\n");
        else fprintf(stderr, "IError\n");
    } else if (!strcmp(op, "delete")) {
        ret = delete(caller_id, command);
        
        if (!ret) printf("Deleted cdr-id\n");
        else fprintf(stderr, "DError\n");
    } else if (!strcmp(op, "find")) {
        ret = find(caller_id, command);

        if (!ret) fprintf(stderr, "No CDRs found\n");
    } else if (!strcmp(op, "lookup")) {
        ret = lookup(callee_id, command);

        if (!ret) fprintf(stderr, "No CDRs found\n");
    } else if (!strcmp(op, "indist")) {
        ret = indist(caller_id, callee_id, command);

        if (!ret) fprintf(stderr, "No indist found\n");
    } else if (!strcmp(op, "topdest")) {
        ret = topdest(caller_id, command);

        if (!ret) fprintf(stderr, "No calls found\n");
    } else if (!strcmp(op, "top")) {
        ret = top(generatedRevenue, command);

        if (!ret) fprintf(stderr, "No calls found\n");
    } else if (!strcmp(op, "bye")) {
        bye(caller_id, callee_id, generatedRevenue);
    } else if (!strcmp(op, "print")) {
        print(caller_id, callee_id, command);
    } else if (!strcmp(op, "dump")) {
        dump(caller_id, callee_id, command);
    } else {
        printf("Operation: %s not recognised. Try again.\n", op);
    }
    free(temp); // free memory allocated for temporary string
}

int getOperationsFromFile(char* operationsFile, HashTable* caller_id, HashTable* callee_id, Heap* generatedRevenue) {
	int error = 0;
	FILE *fp;
	fp = fopen(operationsFile, "r");	// (try to) open operations file
	if (fp == NULL) {		// in case it couldn't open it, report the problem
		fprintf(stderr, "ERROR! Unable to open operationsFile file named %s\n", operationsFile);
		error = -1;
		return error;
	}

	char* command = NULL;   // forces getline to allocate with malloc
    size_t len = 0; // ignored when line=NULL
    ssize_t read;
    while ((read = getline(&command, &len, fp)) != -1) {
        manage_command(caller_id, callee_id, generatedRevenue, command);

        printf("\n");   // for aesthetic reasons
    }
    free(command);  // free memory allocated by getline
	
	return error;
}

/*
considering that a date is in the format DDMMYYYY
I return 1 if date1 precedes date2 ("right order" considering the parameters' order)
	-//- 0 if date1 is the same as date2
	-//- -1 if date2 precedes date1 ("wrong order" considering the parameters' order) 
*/
int compare_dates(char* date1, char* date2) {
	char *day1 = NULL, *day2 = NULL, *month1 = NULL, *month2 = NULL, *year1 = NULL, *year2 = NULL;
	int ret = 0, d1, d2, m1, m2, y1, y2;

	year1 = (char*)malloc(5*sizeof(char));		// isolate date1's year part
	memcpy(year1, &date1[4], 5*sizeof(char));
	year1[4] = '\0';
	year2 = (char*)malloc(5*sizeof(char));		// isolate date2's year part
	memcpy(year2, &date2[4], 5*sizeof(char));
	year2[4] = '\0';
	y1 = atoi(year1);
	y2 = atoi(year2);
	
	if (y1 < y2) ret = 1;
	else if (y1 > y2) ret = -1;
	else {	// if y1 == y2, then we need to check further
		month1 = (char*)malloc(3*sizeof(char));		// isolate date1's month part
		memcpy(month1, &date1[2], 3*sizeof(char));
		month1[2] = '\0';
		month2 = (char*)malloc(3*sizeof(char));		// isolate date2's month part
		memcpy(month2, &date2[2], 3*sizeof(char));
		month2[2] = '\0';
		m1 = atoi(month1);
		m2 = atoi(month2);
		
		if (m1 < m2) ret = 1;
		else if (m1 > m2) ret = -1;
		else {	// if m1 == m2, then we need to check further
			day1 = (char*)malloc(3*sizeof(char));		// isolate date1's day part
			memcpy(day1, date1, 3*sizeof(char));
			day1[2] = '\0';
			day2 = (char*)malloc(3*sizeof(char));		// isolate date2's day part
			memcpy(day2, date2, 3*sizeof(char));
			day2[2] = '\0';
			d1 = atoi(day1);
			d2 = atoi(day2);
			
			if (d1 < d2) ret = 1;
			else if (d1 > d2) ret = -1;
			// else, if d1 == d2, go to the end of the function to return 0
		}
	}

	free(year1);	// free any memory that might have been dynamically allocated
	free(year2);
	free(month1);
	free(month2);
	free(day1);
	free(day2);

	return ret;
}

/*
considering that a time is in the format HH:MM
I return 1 if time1 precedes time2 ("right order" considering the parameters' order)
	-//- 0 if time1 is the same as time2
	-//- -1 if time2 precedes time1 ("wrong order" considering the parameters' order) 
*/
int compare_times(char* time1, char* time2) {
	char *hour1 = NULL, *hour2 = NULL, *minute1 = NULL, *minute2 = NULL;
	int ret = 0, h1, h2, m1, m2;

	hour1 = (char*)malloc(3*sizeof(char));	// isolate time1's hour part
	memcpy(hour1, time1, 2*sizeof(char));
	hour1[2] = '\0';
	hour2 = (char*)malloc(3*sizeof(char));	// isolate time2's hour part
	memcpy(hour2, time2, 2*sizeof(char));
	hour2[2] = '\0';
	h1 = atoi(hour1);
	h2 = atoi(hour2);

	if (h1 < h2) ret = 1;
	else if (h1 > h2) ret = -1;
	else {	// if hour1 == hour2, then we need to check further
		minute1 = (char*)malloc(3*sizeof(char));	// isolate time1's minute part
		memcpy(minute1, &time1[3], 2*sizeof(char));
		minute1[2] = '\0';
		minute2 = (char*)malloc(3*sizeof(char));	// isolate time2's minute part
		memcpy(minute2, &time2[3], 2*sizeof(char));
		minute2[2] = '\0';
		m1 = atoi(minute1);
		m2 = atoi(minute2);

		if (m1 < m2) ret = 1;
		else if (m1 > m2) ret = -1;
		// else, if m1 == m2, go to the end of the function to return 0
	}

	free(hour1);	// free any memory that might have been dynamically allocated
	free(hour2);
	free(minute1);
	free(minute2);

	return ret;
}

int filter_contacts(char* caller1, char* caller2, HashTable* caller_id, HashTable* callee_id, char*** contacts1, int size1, char*** contacts2, int size2, char*** finalcontacts) {
	int i, j, count = 0;	// "count" is the number of the returned phone-numbers (our result)

	int temp = 0;
	char** tempcontacts = NULL;	// our common numbers (temp contact-list)
	// find the common numbers between the two contact-lists
	for (i = 0; i < size1; i++) { // iterate the first contact-list
		// if current number is not any of the "indist" command's "callers" and is in the other contact-list
		if ((strcmp((*contacts1)[i], caller1) != 0) && (strcmp((*contacts1)[i], caller2) != 0) && isInContacts((*contacts1)[i], size2, *contacts2)) {
			tempcontacts = realloc(tempcontacts, (temp+1)*sizeof(char*));	// allocate the necessary memory...
			tempcontacts[temp] = malloc(strlen((*contacts1)[i])+1);			// ...and then save the number in our temp contact-list
			strcpy(tempcontacts[temp], (*contacts1)[i]);
			temp++;
		}
	}

	int assoc;	// our flag, indicating discovered association between current contact and any other contact
	for (i = 0; i < temp; i++) {
		if (tempcontacts[i] != NULL) {
			assoc = 0;	// no association discovered, yet
			for (j = 0; j < temp; j++) {
				if (i != j && tempcontacts[j] != NULL && associated(callee_id, tempcontacts[i], tempcontacts[j])) {
					// if they have communicated (are associated) with each other
					free(tempcontacts[j]);	// free contact, so that it doesn't bother us again
					tempcontacts[j] = NULL;
					assoc = 1;	// association discovered
				}
			}
			if (assoc) {
				free(tempcontacts[i]);	// free contact, so that it doesn't bother us again
				tempcontacts[i] = NULL;
			}
		}
	}

	// construct our final contact-list (our result)
	for (i = 0; i < temp; i++) {
		if (tempcontacts[i] != NULL) {	// any contact that has passed the above tests is part of our result, so we copy it in our final contact-list
			*finalcontacts = (char**)realloc(*finalcontacts, (count+1)*sizeof(char*));
			(*finalcontacts)[count] = (char*)malloc(strlen(tempcontacts[i])+1);
			strcpy((*finalcontacts)[count], tempcontacts[i]);
			count++;
		}
	}

	// free any dynamically allocated memory
	for (i = 0; i < temp; i++) free(tempcontacts[i]);
	free(tempcontacts);

	return count;
}

/*
Take a "contacts' list" (array of phone numbers) and a phone number and report whether this phone number is included inside the list
return 1 for positive answer
return 0 for negative answer
*/
int isInContacts(char* number, int size, char** contacts) {
	int i;
	for (i = 0; i < size; i++)
		if (contacts[i] != NULL && !strcmp(contacts[i], number))
			return 1;	// we have a positive answer

	return 0;	// not found above
}

/*
Take two phone numbers and find out if one has contacted the other
*/
int associated(HashTable* callee_id, char* number1, char* number2) {
	char **number1callers = NULL, **number2callers = NULL;
	int ret = 0, c1 = 0, c2 = 0, i;

    c1 = contacts_in_hashtable(callee_id, number1, &number1callers, 0);	// numbers that contacted number1
    if (isInContacts(number2, c1, number1callers)) ret = 1;
    else {
	    c2 = contacts_in_hashtable(callee_id, number2, &number2callers, 0);	// numbers that contacted number2
	    if (isInContacts(number1, c2, number2callers)) ret = 1;
    }

    // free the dynamically allocated contact-lists
    for (i = 0; i < c1; i++) free(number1callers[i]);
    free(number1callers);
	for (i = 0; i < c2; i++) free(number2callers[i]);
    free(number2callers);

	return ret;
}

int find_top_destination(char** contacts, int size) {
	char **destinations = NULL;
	int *count = NULL, c = 0, i, max, m, total = 0;

	// count frequency of each occurency
	for (i = 0; i < size; i++) {	// go through the contacts list
		if (codeNotIncluded(contacts[i], destinations, c)) {	// if we haven't counted this code before
			destinations = realloc(destinations, (c+1)*sizeof(char*));
			destinations[c] = malloc(4*sizeof(char));
			memcpy(destinations[c], contacts[i], 3*sizeof(char));
			destinations[c][3] = '\0';
			
			count = realloc(count, (c+1)*sizeof(int));
			count[c] = 1;
			
			c++;
		} else {	// if we have counted before
			char country[4] = "---";
			memcpy(country, contacts[i], 3*sizeof(char));	// isolate country-code
			country[3] = '\0';
			
			int j;
			for (j = 0; j < c; j++) {
				if (!strcmp(destinations[j],country)) {
					count[j]++;
					break;
				}
			}
		}
	}

	max = count[0];		// find max number of calls at a certain destination
	m = 0;
	for (i = 0; i < c; i++) {
		if (count[i] > max) {
			max = count[i];
			m = i;
		}
	}

	// print all top-destinations and count how many you printed
	if (max != 0) {
		for (i = 0; i < c; i++) {
			if (count[i] == max) {
				assert(count[i] == count[m]);	// for debugging
				// we print our results
				printf("Country code: %s, Calls made: %d\n", destinations[i], count[i]);
				total++;
			}
		}
	}

	for (i = 0; i < c; i++) free(destinations[i]);
	free(destinations);
	free(count);

	return total;
}

int codeNotIncluded(char* contact, char** destinations, int size) {
	int i;
	char country[4] = "---";	// initialized, to avoid memory errors in strcmp
	strncpy(country, contact, 3);

	for (i = 0; i < size; i++)
		if (!strcmp(country, destinations[i]))
			return 0;	// negative, included

	return 1;	// positive, not included
}