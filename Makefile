all:
	gcc -Wall -g -O0 -o table main.c
clean:
	rm -rf table
