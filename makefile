CC = gcc
CFLAGS =
OBJS = main.o utility.o scan.o parse.o

cMinusCompiler.exec: $(OBJS)
	$(CC) $(CFLAGS) -o cMinusCompiler $(OBJS)

main.o: main.c globals.h utility.h scan.h parse.h
	$(CC) $(CFLAGS) -c main.c

utility.o: utility.c utility.h globals.h
	$(CC) $(CFLAGS) -c utility.c

scan.o: scan.c scan.h globals.h
	$(CC) $(CFLAGS) -c scan.c

parse.o: parse.c parse.h globals.h
	$(CC) $(CFLAGS) -c parse.c

clean:
	rm cMinusCompiler
	rm main.o
	rm utility.o
	rm scan.o
	rm parse.o