# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include external/libs/makefiles/platforms.mk
include external/libs/makefiles/libpaths.mk
include external/libs/bflibc/makefiles/checksum.mk
include external/libs/bflibc/makefiles/uuid.mk

DIRS = bin
CTOOLS = getsize mytime getcount ip4domain passgen getpath organize
CPPTOOLS = search
BASHTOOLS = rsatool listtools
RUSTTOOLS = stopwatch num2bin num2hex cpy
GOTOOLS = 
LIBCPATH = external/libs/$(BF_LIB_RPATH_RELEASE_C) 
LIBRUSTPATH = external/libs/bin/release/rust/release/libbfrust.rlib

## Compiler definitions
CC = gcc
CPPC = g++
RUSTC = rustc
GO = go

## Compile Flags

# Includes
CFLAGS += -Iexternal/libs/$(BF_LIB_RPATH_RELEASE) $(LIBCPATH) $(LDFLAGS) $(BF_LIB_C_UUID_FLAGS)
CPPFLAGS += $(CFLAGS)
RUSTFLAGS += --extern bflib=$(LIBRUSTPATH)
GOFLAGS = 

.PHONY: $(CTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS) lib

build: $(DIRS) $(CTOOLS) $(CPPTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS) check

update-dependencies:
	cd ./external/libs && git pull && make

$(DIRS):
	mkdir -p $@/

# TODO: how to make target build only when deps change

$(CTOOLS): % : src/%/main.c
	$(CC) -o bin/$@ $< $(CFLAGS)

$(CPPTOOLS): % : src/%/main.cpp
	$(CPPC) -o bin/$@ $< $(CFLAGS)

check: % : src/%/main.c
	$(CC) -o bin/$@ $< -lpthread $(CFLAGS) $(BF_LIB_C_CHECKSUM_FLAGS) 

$(RUSTTOOLS): % : src/%/main.rs
	$(RUSTC) -o bin/$@ $< $(RUSTFLAGS)

$(GOTOOLS): % : src/%/main.go
	$(GO) build -o bin/$@ $< $(GOFLAGS)

$(BASHTOOLS): % : src/%/script.sh
	@bash -n $<
	@cp -afv $< bin/$@
	@chmod 755 bin/$@

setup: $(DIRS)

clean:
	rm -rfv $(DIRS)

$(LIBCPATH) : lib
$(LIBRUSTPATH): lib
lib:
	cd external/libs && make

debug: CFLAGS += -g
debug: RUSTFLAGS += -g --extern bflib=$(LIBRUSTPATH)
debug:
	@echo "Debug builds...";

