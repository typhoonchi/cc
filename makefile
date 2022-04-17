CC = gcc
CFLAGS =
OBJS = main.o utility.o scan.o parse.o

cMinusCompiler.exec: $(OBJS)
	$(CC) $(CFLAGS) -o cMinusCompiler $(OBJS)

main.o: main.c globals.h utility.h scan.h parse.h
	$(CC) $(CFLAGS) -c main.c
