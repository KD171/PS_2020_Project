CC=gcc

all: cliv serv_br serv

cliv: batery_read.c 
	gcc -o cliv batery_read.c 

serv_br: multi_br.c 
	gcc -o serv_br multi_br.c 

serv: multi.c
	gcc -o serv multi.c
clean:
	rm cliv_bat serv_br serv
