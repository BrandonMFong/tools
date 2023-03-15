# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include external/libs/makefiles/platforms.mk
include external/libs/makefiles/libpaths.mk

DIRS = bin
CTOOLS = getsize mytime fsinfo getcount netinfo getip passgen getpath organize
BASHTOOLS = rsatool
RUSTTOOLS = stopwatch num2bin num2hex
GOTOOLS = numshift
LIBCPATH = external/libs/$(BF_LIB_RPATH_RELEASE_C) 
LIBRUSTPATH = external/libs/bin/release/rust/release/libbfrust.rlib

## Compiler definitions
CC = gcc
CPP = g++
RUSTC = rustc
GO = /usr/local/go/bin/go

## Compile Flags

# Includes
CFLAGS += -Iexternal/libs/$(BF_LIB_RPATH_RELEASE) $(LIBCPATH)
RUSTFLAGS += -C opt-level=3 --extern bflib=$(LIBRUSTPATH)
GOFLAGS = 

.PHONY: $(CTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS)
all: $(DIRS) $(CTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS)

$(DIRS):
	mkdir -p $@/

# TODO: how to make target build only when deps change

$(CTOOLS): % : src/%/main.c
	$(CC) -o bin/$@ $< $(CFLAGS)

$(RUSTTOOLS): % : src/%/main.rs
	$(RUSTC) -o bin/$@ $< $(RUSTFLAGS)

$(GOTOOLS): % : src/%/main.go
	$(GO) build -o bin/$@ $< $(GOFLAGS)

$(BASHTOOLS): % : src/%/script.sh
	@cp -afv $< bin/$@
	@chmod 755 bin/$@

setup: $(DIRS)

clean:
	rm -rfv $(DIRS)

$(LIBCPATH) : lib
$(LIBRUSTPATH): lib
lib:
	cd external/libs && make

