##########################################
#					 #
#	Makefile for syspro project1. 	 #
#					 #
##########################################

OBJS	= hash.o graph.o main.o bank.o stack.o
SOURCE	= hash.c graph.c main.c bank.c stack.c
HEADER	= types.h graph.h hash.h bank.h stack.h
MISC    = README Makefile
TAR     = release.tar
OUT 	= elegxos
CC	= gcc
FLAGS 	= -g -c -std=c99 -Wall
LIBS 	= 

graph: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LIBS)

main.o: main.c $(HEADER)
	$(CC) $(FLAGS) main.c

graph.o: graph.c graph.h types.h stack.h
	$(CC) $(FLAGS) graph.c

hash.o: hash.c hash.h types.h graph.h
	$(CC) $(FLAGS) hash.c

bank.o: bank.c bank.h types.h graph.h
	$(CC) $(FLAGS) bank.c

stack.o: stack.c stack.h
	$(CC) $(FLAGS) stack.c

clean:
	rm -f $(OBJS) $(OUT)

loc:
	wc -l $(SOURCE) $(HEADER)

release:
	tar -cvf $(TAR) $(SOURCE) $(HEADER) $(MISC)
