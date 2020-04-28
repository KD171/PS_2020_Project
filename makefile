CC=gcc

all: agent menager

agent: agent.c 
	gcc -pthread -o agent agent.c 

menager: menager.c 
	gcc -pthread -o menager menager.c

clean:
	rm agent menager
