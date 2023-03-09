# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include external/libs/makefiles/platforms.mk
include external/libs/makefiles/libpaths.mk

CTOOLS = getsize mytime fsinfo getcount netinfo getip passgen getpath organize
BASHTOOLS = rsatool
RUSTTOOLS = stopwatch num2bin num2hex

## Compiler definitions
CC = gcc
CPP = g++
RUSTC = rustc

## Compile Flags

# Includes
CFLAGS += -I. -Iexternal/libs/$(BF_LIB_RPATH_RELEASE) external/libs/$(BF_LIB_RPATH_RELEASE_C)
RUSTFLAGS += -C opt-level=3 --extern bflib=external/libs/bin/release/rust/release/libbfrust.rlib

all: setup $(CTOOLS) $(BASHTOOLS) $(RUSTTOOLS)

setup:
	mkdir -p bin/
	mkdir -p build/

$(CTOOLS):
	$(CC) -o bin/$@ src/$@/main.c $(CFLAGS)

$(RUSTTOOLS):
	$(RUSTC) -o bin/$@ src/$@/main.rs $(RUSTFLAGS)

$(BASHTOOLS):
	@cp -afv src/$@/script.sh bin/$@
	@chmod 755 bin/$@

clean:
	rm -rfv bin
	rm -rfv build

lib:
	cd external/libs && make

