# author: Brando
# date: 6/2/22

all: setup getsize mytime fsinfo

setup:
	mkdir -p bin/

getsize: src/getsize/main.c src/cutils/utilities.c
	gcc -o bin/getsize src/getsize/main.c src/cutils/utilities.c -I.

mytime: src/mytime/main.c src/cutils/utilities.c
	gcc -o bin/mytime src/mytime/main.c src/cutils/utilities.c -I.

fsinfo: src/fsinfo/main.c src/cutils/utilities.c
	gcc -o bin/fsinfo src/fsinfo/main.c src/cutils/utilities.c -I.
clean:
	rm -rfv bin/*
