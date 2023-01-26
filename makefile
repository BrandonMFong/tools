# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include lib/makefiles/platforms.mk

## Compiler definitions
CC = gcc
CPP = g++

## Compile Flags

# Includes
CFLAGS += -I. -Ilib/ lib/bin/c/clib.a

all: setup getsize mytime fsinfo getcount netinfo getip passgen getpath createfile organize

setup:
	mkdir -p bin/
	mkdir -p build/

getsize:
	$(CC) -o bin/getsize src/getsize/main.c $(CFLAGS)

mytime:
	$(CC) -o bin/mytime src/mytime/main.c $(CFLAGS)

fsinfo:
	$(CC) -o bin/fsinfo src/fsinfo/main.c $(CFLAGS)

getcount:
	$(CC) -o bin/getcount src/getcount/main.c $(CFLAGS)

netinfo:
	$(CC) -o bin/netinfo src/netinfo/main.c $(CFLAGS)

getip:
	$(CC) -o bin/getip src/getip/main.c $(CFLAGS)

passgen:
	$(CC) -o bin/passgen src/passgen/main.c $(CFLAGS)

getpath:
	$(CC) -o bin/getpath src/getpath/main.c $(CFLAGS)

createfile:
	$(CC) -o bin/createfile src/createfile/main.c $(CFLAGS)

organize:
	$(CC) -o bin/organize src/organize/main.c $(CFLAGS)

clean:
	rm -rfv bin
	rm -rfv build

