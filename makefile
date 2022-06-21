# author: Brando
# date: 6/2/22

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

all: setup getsize mytime fsinfo getcount netinfo

setup:
	mkdir -p bin/

getsize: src/getsize/main.c lib/clib/clib.c
	gcc -o bin/getsize src/getsize/main.c lib/clib/clib.c -I. -Ilib/ $(CCFLAGS)

mytime: src/mytime/main.c lib/clib/clib.c
	gcc -o bin/mytime src/mytime/main.c lib/clib/clib.c -I. -Ilib/ $(CCFLAGS)

fsinfo: src/fsinfo/main.c lib/clib/clib.c
	gcc -o bin/fsinfo src/fsinfo/main.c lib/clib/clib.c -I. -Ilib/ $(CCFLAGS)

getcount: src/getcount/main.c lib/clib/clib.c
	gcc -o bin/getcount src/getcount/main.c lib/clib/clib.c -I. -Ilib/ $(CCFLAGS)

netinfo: src/netinfo/main.c lib/clib/clib.c
	gcc -o bin/netinfo src/netinfo/main.c lib/clib/clib.c -I. -Ilib/ $(CCFLAGS)

clean:
	rm -rfv bin/*
