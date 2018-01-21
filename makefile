CC = gcc
CFLAGS = -Wall
LIBS = -lm
EXEC = werhauz
OBJECTS = main.o functions.o commands.o structures.o heap.o
SOURCES = main.c functions.c commands.c structures.c heap.c
HEADERS = header.h functions.h commands.h structures.h heap.h

manager: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS) $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

functions.o: functions.c
	$(CC) $(CFLAGS) -c functions.c

commands.o: commands.c
	$(CC) $(CFLAGS) -c commands.c

structures.o: structures.c
	$(CC) $(CFLAGS) -c structures.c

heap.o: heap.c
	$(CC) $(CFLAGS) -c heap.c

.PHONY: clean

clean:
	rm -f $(EXEC) $(OBJECTS)

count:
	wc $(SOURCES) $(HEADERS)