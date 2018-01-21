#include "header.h"

int h1NumOfEntries, h2NumOfEntries, bucketSize, connectionSize;

int main (int argc, char *argv[]) {
    /* SET UP BASED ON COMMAND LINE'S ARGUMENTS */
    if (argc < 7) {
        fprintf(stderr, "ERROR! Insufficient number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    int h1flag = 0, h2flag = 0, sflag = 0, oflag = 0, cflag = 0;
    char *operationsFile = NULL, *configFile = NULL;

    int i;
    for (i = 1; i < argc-1; i += 2) {   // go through the program's arguments list
        if (!oflag && !strcmp(argv[i], "-o")) { // the first time we find the operations file flag
            oflag = 1;
            operationsFile = (char*)malloc(strlen(argv[i+1])+1);
            strcpy(operationsFile, argv[i+1]);
        } else if (!h1flag && !strcmp(argv[i], "-h1")) { // the first time we find the h1NumOfEntries flag
            h1flag = 1;
            h1NumOfEntries = atoi(argv[i+1]);
        } else if (!h2flag && !strcmp(argv[i], "-h2")) { // the first time we find the h2NumOfEntries flag
            h2flag = 1;
            h2NumOfEntries = atoi(argv[i+1]);
        } else if (!sflag && !strcmp(argv[i], "-s")) { // the first time we find the bucketSize flag
            sflag = 1;
            bucketSize = atoi(argv[i+1]);
        } else if (!cflag && !strcmp(argv[i], "-c")) { // the first time we find the configurations file flag
            cflag = 1;
            configFile = (char*)malloc(strlen(argv[i+1])+1);
            strcpy(configFile, argv[i+1]);
        } else {
            fprintf(stderr, "ERROR! Bad argument formation!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!h1flag || !h2flag || !sflag) {   // test if all the neccessary flags have been included
        fprintf(stderr, "ERROR! Arguments missing!\n");
        exit(EXIT_FAILURE);
    }

    if (h1NumOfEntries <= 0 || h2NumOfEntries <= 0) {   // test for correct hashtable size arguments
        fprintf(stderr, "ERROR! Both hashtables need to have size greater than zero (0).\n");
        exit(EXIT_FAILURE);
    }

    if (bucketSize < (sizeof(int)+sizeof(long int)+sizeof(Connection*)+sizeof(Bucket*))) {   // we need space for a Bucket with at least one entry
        fprintf(stderr, "ERROR! Insufficient bucket size.\n");
        exit(EXIT_FAILURE);
    }

    connectionSize = 2*bucketSize;  // each connection-structure will have the double size of a bucket-structure

    // our charging configurations array
    double **config;
    config = malloc(5*sizeof(double*));
    for (i = 0; i < 5; i++) config[i] = malloc(3*sizeof(double));
    
    if (cflag) {
        int ret = getConfigurations(configFile, config);
        if (ret == -1) {  // we need space for a Connection with at least one entry
            fprintf(stderr, "ERROR! Bad input from configuration file! Try again.\n");
            for (i = 0; i < 5; i++) free(config[i]);
            free(config);
            free(operationsFile);
            if (cflag) free(configFile);
            return ret;
        }
    } else {    // assign default configuration values
        config[0][0] = 0; config[0][1] = 0; config[0][2] = 1;
        config[1][0] = 1; config[1][1] = 1; config[1][2] = 0.5;
        config[2][0] = 1; config[2][1] = 2; config[2][2] = 1;
        config[3][0] = 2; config[3][1] = 1; config[3][2] = 0.25;
        config[4][0] = 2; config[4][1] = 2; config[4][2] = 0.4;
    }
    // printf("We have: %d Hashtable1NumOfEntries, %d Hashtable2NumOfEntries, %d BucketSize, %d ConnectionSize and %s for operations\n", h1NumOfEntries, h2NumOfEntries, bucketSize, connectionSize, operationsFile);
    // if (cflag) printf("We have: %s for configFile\n", configFile);

    HashTable caller_id, callee_id;
    Heap generatedRevenue;
    initialize_hashtable(&caller_id, "caller");
    initialize_hashtable(&callee_id, "callee");
    /*INITIALIZE HEAP*/
    initialize_heap(&generatedRevenue, &config);

    /* FILE'S OPERATIONS */
    if (oflag) {
        int ret = getOperationsFromFile(operationsFile, &caller_id, &callee_id, &generatedRevenue);
        if (ret == -1)  // check for input error
            fprintf(stderr, "ERROR! Bad input from operations file! Quit (Ctrl^D) to try again.\n");
    }

    /* COMMAND LINE'S OPERATIONS */
    // for the input method, I consulted: http://stackoverflow.com/questions/7709452/how-to-read-string-from-keyboard-using-c
    char* command = NULL;   // forces getline to allocate with malloc
    size_t len = 0; // ignored when line=NULL
    ssize_t read;
    printf("\nEnter a command below. Press [ctrl+d] to quit\n");
    while ((read = getline(&command, &len, stdin)) != -1) {
        manage_command(&caller_id, &callee_id, &generatedRevenue, command);

        printf("\n");   // for aesthetic reasons
    }
    free(command);  // free memory allocated by getline
    printf("Terminating...\nGoodbye!\n");

    // free-dynamically allocated memory
    for (i = 0; i < 5; i++) free(config[i]);
    free(config);
    free(operationsFile);
    if (cflag) free(configFile);
    clear_hashtable(&caller_id, "");
    clear_hashtable(&callee_id, "-cdr");
    /*CLEAR HEAP*/
    clear_heap(&generatedRevenue);

    return 0;
}