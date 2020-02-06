all:
	gcc -Wall -g -O0 -o table table.c demo.c
clean:
	rm -rf table
