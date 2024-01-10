# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include external/libs/makefiles/platforms.mk
include external/libs/makefiles/libpaths.mk
include external/libs/bflibc/makefiles/checksum.mk
include external/libs/bflibc/makefiles/uuid.mk

BIN_PATH = bin/release
DIRS = bin $(BIN_PATH)
CTOOLS = getsize mytime getcount ip4domain passgen getpath organize search
CPPTOOLS =
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


$(CTOOLS): % : src/%/main.c
	$(CC) -o $(BIN_PATH)/$@ $< $(CFLAGS)

$(CPPTOOLS): % : src/%/main.cpp
	$(CPPC) -o $(BIN_PATH)/$@ $< $(CFLAGS)

check: % : src/%/main.c
	$(CC) -o $(BIN_PATH)/$@ $< -lpthread $(CFLAGS) $(BF_LIB_C_CHECKSUM_FLAGS) 

$(RUSTTOOLS): % : src/%/main.rs
	$(RUSTC) -o $(BIN_PATH)/$@ $< $(RUSTFLAGS)

$(GOTOOLS): % : src/%/main.go
	$(GO) build -o $(BIN_PATH)/$@ $< $(GOFLAGS)

$(BASHTOOLS): % : src/%/script.sh
	@bash -n $<
	@cp -afv $< $(BIN_PATH)/$@
	@chmod 755 $(BIN_PATH)/$@

setup: $(DIRS)

clean:
	rm -rfv $(DIRS)

$(DIRS):
	mkdir -p $@/

debug: CFLAGS += -g
debug: BIN_PATH = bin/debug
debug: DIRS = bin/debug
debug: RUSTFLAGS += -g --extern bflib=$(LIBRUSTPATH)
debug: $(DIRS) build

lib-update:
	cd ./external/libs && git pull && make

lib:
	cd external/libs && make

