# author: Brando
# date: 6/2/22

getsize: src/getsize/main.c src/cutils/utilities.c
	gcc -o bin/getsize src/getsize/main.c src/cutils/utilities.c -I.


