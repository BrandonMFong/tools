# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include lib/makefiles/platforms.mk

CTOOLS = getsize mytime fsinfo getcount netinfo getip passgen getpath createfile organize
BASHTOOLS = rsatool
RUSTTOOLS = stopwatch

## Compiler definitions
CC = gcc
CPP = g++
RUSTC = rustc

## Compile Flags

# Includes
CFLAGS += -I. -Ilib/ lib/bin/c/clib.a

all: setup $(CTOOLS) $(BASHTOOLS) $(RUSTTOOLS)

setup:
	mkdir -p bin/
	mkdir -p build/

$(CTOOLS):
	$(CC) -o bin/$@ src/$@/main.c $(CFLAGS)

$(RUSTTOOLS):
	$(RUSTC) -o bin/$@ src/$@/main.rs

$(BASHTOOLS):
	@cp -afv src/$@/script.sh bin/$@
	@chmod 755 bin/$@

clean:
	rm -rfv bin
	rm -rfv build

