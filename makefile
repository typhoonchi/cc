CC = gcc
CFLAGS =
OBJS = main.o utility.o scanner.o parser.o code.o

cMinusCompiler.exec: $(OBJS)
	$(CC) $(CFLAGS) -o cMinusCompiler $(OBJS)

main.o: main.c globals.h utility.h scanner.h parser.h code.h
	$(CC) $(CFLAGS) -c main.c

utility.o: utility.c utility.h globals.h
	$(CC) $(CFLAGS) -c utility.c

scan.o: scanner.c scanner.h globals.h
	$(CC) $(CFLAGS) -c scanner.c

parse.o: parser.c parser.h globals.h
	$(CC) $(CFLAGS) -c parser.c

code.o: code.c code.h globals.h
	$(CC) $(CFLAGS) -c code.c

clean:
	rm cMinusCompiler
	rm main.o
	rm utility.o
	rm scanner.o
	rm parser.o
	rm code.o