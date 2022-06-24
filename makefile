# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Compiler definitions
CC = gcc

## Compile Flags

# Includes
CCFLAGS += -I. -Ilib/

# Determine the OS
ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -D OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif

# Warnings
CCFLAGS += -Wall

all: setup clib getsize mytime fsinfo getcount netinfo getip

setup:
	mkdir -p bin/
	mkdir -p build/

clib:
	$(CC) -c -o build/clib.o lib/clib/clib.c $(CCFLAGS)

getsize:
	$(CC) -o bin/getsize src/getsize/main.c build/clib.o $(CCFLAGS)

mytime:
	$(CC) -o bin/mytime src/mytime/main.c build/clib.o $(CCFLAGS)

fsinfo:
	$(CC) -o bin/fsinfo src/fsinfo/main.c build/clib.o $(CCFLAGS)

getcount:
	$(CC) -o bin/getcount src/getcount/main.c build/clib.o $(CCFLAGS)

netinfo:
	$(CC) -o bin/netinfo src/netinfo/main.c build/clib.o $(CCFLAGS)

getip:
	$(CC) -o bin/getip src/getip/main.c build/clib.o $(CCFLAGS)

clean:
	rm -rfv bin
	rm -rfv build
