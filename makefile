# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include external/libs/makefiles/platforms.mk
include external/libs/makefiles/libpaths.mk
include external/libs/bflibc/makefiles/checksum.mk
include external/libs/bflibc/makefiles/uuid.mk

BIN_PATH = bin/release
DIRS = $(BIN_PATH) bin
CTOOLS = getsize mytime getcount ip4domain passgen getpath organize search check
CPPTOOLS =
BASHTOOLS = rsatool listtools
RUSTTOOLS = stopwatch num2bin num2hex cpy
GOTOOLS = 
LIBCPATH = external/libs/$(BF_LIB_RPATH_RELEASE_C) 
LIBRUSTPATH = external/libs/bin/release/rust/release/libbfrust.rlib
TESTING_MACRO = TESTING
DEBUG_MACRO = DEBUG

## Compiler definitions
CC = gcc
CPPC = g++
RUSTC = rustc
GO = go

## Compiler flags
CFLAGS += -Iexternal/libs/$(BF_LIB_RPATH_RELEASE) $(LIBCPATH) $(LDFLAGS) $(BF_LIB_C_UUID_FLAGS)
CPPFLAGS += $(CFLAGS)
RUSTFLAGS += --extern bflib=$(LIBRUSTPATH)
GOFLAGS = 

## Tool Specific
check_deps = -lpthread $(BF_LIB_C_CHECKSUM_FLAGS) 

.PHONY: $(CTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS) lib

build: $(DIRS) $(CTOOLS) $(CPPTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS)

setup: $(DIRS)

clean:
	rm -rfv $(DIRS)

$(DIRS):
	mkdir -p $@/

lib-update:
	cd ./external/libs && git pull && make

lib:
	cd external/libs && make

$(CTOOLS): % : src/%/main.c
	$(CC) -o $(BIN_PATH)/$@ $< $(CFLAGS) $($@_deps)

$(CPPTOOLS): % : src/%/main.cpp
	$(CPPC) -o $(BIN_PATH)/$@ $< $(CFLAGS)

$(RUSTTOOLS): % : src/%/main.rs
	$(RUSTC) -o $(BIN_PATH)/$@ $< $(RUSTFLAGS)

$(GOTOOLS): % : src/%/main.go
	$(GO) build -o $(BIN_PATH)/$@ $< $(GOFLAGS)

$(BASHTOOLS): % : src/%/script.sh
	@bash -n $<
	@cp -afv $< $(BIN_PATH)/$@
	@chmod 755 $(BIN_PATH)/$@

## Debug config
debug-setup:
	mkdir -p bin/debug/

debug: CFLAGS += -g -D$(DEBUG_MACRO)
debug: CPPFLAGS += -g -D$(DEBUG_MACRO)
debug: RUSTFLAGS += -g --extern bflib=$(LIBRUSTPATH)
debug: BIN_PATH = bin/debug
debug: debug-setup build

## Test config
test-setup:
	mkdir -p bin/test/
test: CFLAGS += -g -D$(TESTING_MACRO)
test: CPPFLAGS += -g -D$(TESTING_MACRO)
test: RUSTFLAGS += -g --extern bflib=$(LIBRUSTPATH)
test: BIN_PATH = bin/test
test: test-setup $(CTOOLS) 
test: TEST_ITEMS = $(wildcard $(BIN_PATH)/*)
test: $(TEST_ITEMS)
	@for test in $(TEST_ITEMS); do \
        ./$$test; \
    done

