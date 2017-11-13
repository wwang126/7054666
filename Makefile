CC=gcc
CFLAGS=-g -Wall -lpthread

all:
	${CC} smallsh.c -o smallsh ${CFLAGS}

smallsh:
	${CC} smallsh.c -o smallsh ${CFLAGS}

clean:
	rm smallsh
