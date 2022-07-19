# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Compiler definitions
CC = gcc
CPP = g++

## Compile Flags

# Includes
CCFLAGS += -I. -Ilib/

# Determine the OS
ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WINDOWS
    CPPFLAGS += -D WINDOWS
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
        CPPFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
            CPPFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
            CPPFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
        CPPFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -D MACOS
        CPPFLAGS += -D MACOS
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
        CPPFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
        CPPFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
        CPPFLAGS += -D ARM
    endif
endif

all: setup getsize mytime fsinfo getcount netinfo getip passgen getpath

setup:
	mkdir -p bin/
	mkdir -p build/

getsize:
	$(CC) -o bin/getsize src/getsize/main.c lib/bin/clib.o $(CCFLAGS)

mytime:
	$(CC) -o bin/mytime src/mytime/main.c lib/bin/clib.o $(CCFLAGS)

fsinfo:
	$(CC) -o bin/fsinfo src/fsinfo/main.c lib/bin/clib.o $(CCFLAGS)

getcount:
	$(CC) -o bin/getcount src/getcount/main.c lib/bin/clib.o $(CCFLAGS)

netinfo:
	$(CC) -o bin/netinfo src/netinfo/main.c lib/bin/clib.o $(CCFLAGS)

getip:
	$(CC) -o bin/getip src/getip/main.c lib/bin/clib.o $(CCFLAGS)

passgen:
	$(CC) -o bin/passgen src/passgen/main.c lib/bin/clib.o $(CCFLAGS)

getpath:
	$(CC) -o bin/getpath src/getpath/main.c lib/bin/clib.o $(CCFLAGS)

clean:
	rm -rfv bin
	rm -rfv build

