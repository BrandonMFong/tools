# author: Brando
# date: 6/2/22

all: getsize mytime 

getsize: src/getsize/main.c src/cutils/utilities.c
	gcc -o bin/getsize src/getsize/main.c src/cutils/utilities.c -I.

mytime: src/mytime/main.c src/cutils/utilities.c
	gcc -o bin/mytime src/mytime/main.c src/cutils/utilities.c -I.


