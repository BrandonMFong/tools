# author: Brando
# date: 6/2/22

all: setup getsize mytime fsinfo getcount netinfo

setup:
	mkdir -p bin/

getsize: src/getsize/main.c lib/clib/clib.c
	gcc -o bin/getsize src/getsize/main.c lib/clib/clib.c -I. -Ilib/

mytime: src/mytime/main.c lib/clib/clib.c
	gcc -o bin/mytime src/mytime/main.c lib/clib/clib.c -I. -Ilib/

fsinfo: src/fsinfo/main.c lib/clib/clib.c
	gcc -o bin/fsinfo src/fsinfo/main.c lib/clib/clib.c -I. -Ilib/

getcount: src/getcount/main.c lib/clib/clib.c
	gcc -o bin/getcount src/getcount/main.c lib/clib/clib.c -I. -Ilib/

netinfo: src/netinfo/main.c lib/clib/clib.c
	gcc -o bin/netinfo src/netinfo/main.c lib/clib/clib.c -I. -Ilib/

clean:
	rm -rfv bin/*
