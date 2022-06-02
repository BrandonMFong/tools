# author: Brando
# date: 6/2/22

getsize: src/getsize/main.c src/cutils/utilities.c
	gcc -o getsize src/getsize/main.c src/cutils/utilities.c -I.


