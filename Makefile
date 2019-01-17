all: perfect.c
	gcc -o perfect -pthread -lm -O3 perfect.c
