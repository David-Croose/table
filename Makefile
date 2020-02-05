all:
	gcc -Wall -g -O0 -o table table.c
clean:
	rm -rf table
