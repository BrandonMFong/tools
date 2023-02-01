# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include lib/makefiles/platforms.mk

CTOOLS = getsize mytime fsinfo getcount netinfo getip passgen getpath createfile organize
BASHTOOLS = rsatool

## Compiler definitions
CC = gcc
CPP = g++

## Compile Flags

# Includes
CFLAGS += -I. -Ilib/ lib/bin/c/clib.a

all: setup $(CTOOLS) $(BASHTOOLS)

setup:
	mkdir -p bin/
	mkdir -p build/

$(CTOOLS):
	$(CC) -o bin/$@ src/$@/main.c $(CFLAGS)

$(BASHTOOLS):
	cp -afv src/$@/script.sh bin/$@
	chmod 755 bin/$@

clean:
	rm -rfv bin
	rm -rfv build

