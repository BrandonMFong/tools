# author: Brando
# date: 6/2/22
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Includes
include external/libs/makefiles/platforms.mk
include external/libs/makefiles/libpaths.mk
include external/libs/bflibc/makefiles/checksum.mk
include external/libs/bflibc/makefiles/uuid.mk

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	BUILD_TYPE=linux
else 
ifeq ($(UNAME_S),Darwin)
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
		BUILD_TYPE=macos-intel
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
		BUILD_TYPE=macos-intel
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
		BUILD_TYPE=macos-arm
    endif
endif
endif
 
BIN_PATH = bin/release
DIRS = $(BIN_PATH) bin
CTOOLS = getsize mytime getcount ip4domain passgen getpath organize search check getinfo
CPPTOOLS = spellcheck
BASHTOOLS = rsatool listtools
RUSTTOOLS = stopwatch cpy
GOTOOLS = define
INSTALL_SCRIPTS = install uninstall install_utils.sh
LIBRUSTPATH = external/libs/bin/release/rust/release/libbfrust.rlib
TESTING_MACRO = TESTING
DEBUG_MACRO = DEBUG
COMPONENTS = $(CTOOLS) $(CPPTOOLS) $(BASHTOOLS) $(RUSTTOOLS) $(GOTOOLS) $(INSTALL_SCRIPTS)

PACKAGE_NAME = tools
PACKAGE_COMPONENTS = $(patsubst %, $(PACKAGE_NAME)/%, $(COMPONENTS))

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

build: release

release: $(COMPONENTS)

setup: $(DIRS)

clean:
	rm -rfv $(DIRS)
	rm -rfv $(PACKAGE_NAME)

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

$(INSTALL_SCRIPTS) : % : scripts/% $(DIRS)
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

package: $(PACKAGE_NAME) $(PACKAGE_COMPONENTS)
	zip -r $(BIN_PATH)/$(PACKAGE_NAME)-$(BUILD_TYPE).zip $(PACKAGE_NAME)
	tar vczf $(BIN_PATH)/$(PACKAGE_NAME)-$(BUILD_TYPE).tar.gz $(PACKAGE_NAME)

$(PACKAGE_NAME):
	mkdir -p $@

$(PACKAGE_NAME)/%: $(BIN_PATH)/%
	@cp -afv $< $(PACKAGE_NAME)

