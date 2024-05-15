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
CTOOLS = getsize mytime getcount ip4domain passgen getpath organize search check getinfo
CPPTOOLS = spellcheck
BASHTOOLS = rsatool listtools
RUSTTOOLS = stopwatch cpy
GOTOOLS = 
LIBRUSTPATH = external/libs/bin/release/rust/release/libbfrust.rlib
TESTING_MACRO = TESTING
DEBUG_MACRO = DEBUG

## Compiler definitions
CC = gcc
CPPC = g++
RUSTC = rustc
GO = go

## Compiler flags
CFLAGS += -Icommon -Iexternal/libs/$(BF_LIB_RPATH_RELEASE) external/libs/$(BF_LIB_RPATH_RELEASE_C) $(BF_LIB_C_UUID_FLAGS)
CPPFLAGS += -std=c++20 -Icommon -Iexternal/libs/$(BF_LIB_RPATH_RELEASE) external/libs/$(BF_LIB_RPATH_RELEASE_CPP) $(BF_LIB_C_UUID_FLAGS)
RUSTFLAGS += --extern bflib=$(LIBRUSTPATH)
GOFLAGS = 

## Tool Specific

# tool: check
check_deps = -lpthread $(BF_LIB_C_CHECKSUM_FLAGS) 

.PHONY: $(CTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS) lib

build: $(CTOOLS) $(CPPTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS)

setup: $(DIRS)

clean:
	rm -rfv $(DIRS)

$(DIRS):
	mkdir -p $@/

lib-update:
	cd ./external/libs && git pull && make

lib:
	cd external/libs && make

$(CTOOLS): % : src/%/main.c $(DIRS)
	$(CC) -o $(BIN_PATH)/$@ $< $(CFLAGS) $($@_deps)

$(CPPTOOLS): % : src/%/main.cpp $(DIRS)
	$(CPPC) -o $(BIN_PATH)/$@ $< $(CPPFLAGS)

$(RUSTTOOLS): % : src/%/main.rs $(DIRS)
	$(RUSTC) -o $(BIN_PATH)/$@ $< $(RUSTFLAGS)

$(GOTOOLS): % : src/%/main.go $(DIRS)
	$(GO) build -o $(BIN_PATH)/$@ $< $(GOFLAGS)

$(BASHTOOLS): % : src/%/script.sh $(DIRS)
	@bash -n $<
	@cp -afv $< $(BIN_PATH)/$@
	@chmod 755 $(BIN_PATH)/$@

## Debug config
debug-setup:
	mkdir -p bin/debug/

debug-clean:
	rm -rfv bin/debug/

debug: CFLAGS += -g -D$(DEBUG_MACRO)
debug: LIBCPPPATH = external/libs/$(BF_LIB_RPATH_DEBUG_CPP) 
debug: CPPFLAGS = -g -D$(DEBUG_MACRO) -std=c++20 -Icommon -Iexternal/libs/$(BF_LIB_RPATH_DEBUG) $(LIBCPPPATH) $(LDFLAGS) $(BF_LIB_C_UUID_FLAGS)
debug: RUSTFLAGS += -g --extern bflib=$(LIBRUSTPATH)
debug: BIN_PATH = bin/debug
debug: debug-setup build

## Test config
test-setup:
	mkdir -p bin/test/

test-clean:
	rm -rfv bin/test/

test: CFLAGS += -g -D$(TESTING_MACRO)
test: CPPFLAGS += -g -D$(TESTING_MACRO)
test: RUSTFLAGS += -g --extern bflib=$(LIBRUSTPATH)
test: BIN_PATH = bin/test
test: test-setup $(CTOOLS) $(CPPTOOLS)
test: TEST_ITEMS = $(wildcard $(BIN_PATH)/*)
test: $(TEST_ITEMS)
	@for test in $(TEST_ITEMS); do \
        ./$$test; \
    done

