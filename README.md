# Project: Project1 - CDR Records Warehouse

Project Developer: Georgios Kamaras - sdi1400058

Course: K24 Systems Programming, Spring 2017

Date: 16/03/2017

Development Platform:
*	GNU/Linux Ubuntu 16.04
*	gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)

## Included Files
*	makefile
*	header.h (my main library-file, links all the program's modules together)
*	commands.h commands.c (the implementation of the commands that my data-warehouse can accept)
*	functions.h functions.c (various functions that I use throughout my program)
*	heap.h heap.c (my Binary Max Heap Structure's implementation)
*	stuctures.h structures.c (my Hash-Table, Bucket, Bucket-Entry, Connection, (CDR) Record structures implementation)
*	main.c (my program's main function)
*	(and README)

## Compilation
Use ```make``` command

## Cleaning
Use ```make clean``` command

## Usage
Use ```./werhauz <desired-flags>``` command e.g. ```./werhauz -o inputs/input1.txt -c configs/charge-config.txt -h1 12 -h2 12 -s 76```.

## Technical Details
*	For my implementation I followed all of the exercise's specifications both in paper and on Piazza.
	My goal was to develop a system able to store a large amount of CDR records and allow them to be accessed
	in  variety of meaningful ways, all that in a efficient way in terms of memory and time consumption.
	To achive this goal, I implemented all the functionality, both mandatory and optional (not keeping
	duplicate records, ability to interact with a configuration file, ability to export the stored information
	in a text file at any time, etc) described in the exercise's specifications.

*	What my program, basically, does is that it stores CDR records in memory and it provides to the user
	two interfaces to access them, which are our hash-tables. The first hash-table, the caller_id, gives
	access to the CDR records from the scope of their originator number (caller) and the second hash-table,
	the callee_id, gives access to the CDR records from the scope of their destination number (callee). Each
	hash-table has as much positions as the user requested from the program's parameters. Each position links
	to a bucket-list. Each bucket (again as big as the user requested) contains an array of bucket-entries.
	Each bucket-entry contains the number to which information it grants us access to and it links to a
	connection-structures-list. Each connection structure contains an array of pointers which point to
	the CDR records stored in memory related to the previous number, either from the caller's or the callee's
	perspective. So, each CDR record stucture, is at any time pointed to by either one or to pointers (one
	coming from the caller_id interface, which may stop existing after a "caller deletion", and another one
	coming from the callee_id interface). I chose this method of storing and referencing because it allowed
	me to store each CDR record only once (no data duplication).

*	Here, I note that as requested the hash-table size requested by the user is considered as quantity of units
	(a unit being a hash-table position), whereas the bucket-size is considered as Bytes. In a similar logic
	I take the freedom to define the size of the connection-structures in Bytes, specifically, having the
	double size of a bucket-structure.

*	Regarding the storing of the CDR records' information, I store cdr_uniq_id, originator_number, destination_number,
	date and init_time as strings (the whole info, including it's formating) and duration, type, tarrif and
	fault_condition as integers.

*	I take as a fact that there are 3 possible CDR record types, let: call (1), data (2), SMS (0) and, 3
	possible CDR record tarrifs, let: hinterland (1), international (call or data) (2), smscost (0). Also,
	I expect SMS to have a fixed cost. For this reason, I store the information regarding the charging method
	(able to be provided by a configuration file, like charge-config.txt given to us as an example) in an
	array of type "double" and with dimensions of 5 rows and 3 columns (columns separated by ";").

*	At the Generated Revenue's Binary Max Heap Structure, I don't regard as "charged" any CDR record with
	fault_condition other than 2XX kind. We don't want to charge calls that never occured, so if fault-condition
	isn't of the 2XX kind, this means that there was a fault and we can assume that the communication never
	happened.

*	At the Generated Revenue's Binary Max Heap Structure, I keep the company's total revenue. So, whenever I am
	searching my top k-percent clients (revenue-creators), the first thing that I do is to find what is the k-percent
	of the company's total revenue. Then I find and print the top clients (in a descending order of paying (contribution
	to company's revenues)) until I reach the k-percent-amount or surpass it (whenever I can't match it exactly).

*	For my implementation of the "indist" command, firstly, I take all the subscribers (their numbers) that have
	either contacted or have been contacted by each one of the two callers and store them in two arrays, one for
	each caller. Then, I pass these two arrays in a "filter" function which produces the final array, which contains
	the information requested by the "indist" command. What my "filter" does, is that it finds the numbers that are
	common (common contacts) in the two initial arrays and it copies them in a temporary array. Then, it searches
	inside this array and whenever it finds two numbers that are associated with each other, it removes them. In the
	end, whatever has left is our result, so it is copied in our final (result) array. Then, the temporary array is
	freed.

*	I have tested my program using the "valgrind" tool at any stage of it's development and it works without
	any memory-management issue ("0 ERRORS" indication).


## External Sources
*	For the implementation of the Heap Insertion algorithm, in my int insert_to_heap(Heap*, HeapNode*);
	function, I followed the algorithms and the instructions presented at:
	https://www.cpp.edu/~ftang/courses/CS241/notes/Building_Heaps_With_Pointers.pdf
	(a classmate posted it on piazza).

*	For the tokenization of strings (mainly commands), which I use thoroughly in my implementation I consulted:
	http://stackoverflow.com/questions/4513316/split-string-in-c-every-white-space

*	For the input method, I consulted:
	http://stackoverflow.com/questions/7709452/how-to-read-string-from-keyboard-using-c


## Contact - feedback
Georgios Kamaras: sdi1400058@di.uoa.gr
