#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h> 
#define MYPORT 9009    
    struct sockaddr_in servaddr;
    int sd;
    int len;
    struct sockaddr_in from;
    char databuf[1024];
    int on;

    void pidof(char* arg){
	char line[1024];
	char line2[1024];
	char line3[1024];
	FILE *cmd = popen(arg,"r");
	fgets(line,1024,cmd);
	pclose(cmd);
	char space[] = " ";
	int x = strcspn(line, space);
	if(x != strlen(line)){
		strncpy(line3, line,x);
		strncpy(line2, line + x + 1, strlen(line) - x);
		int killme3 = atoi(line3);
		int killme2 = atoi(line2);
		if(killme3 == getpid()){
			kill(killme2,SIGKILL);	
		}else{
			kill(killme3,SIGKILL);
		}
		sleep(1);
	}
}

    int main(int argc, char *argv[])
    {
		char* arg[] = {"pidof ./serv_br"};
		pidof(arg[0]);
        int len = sizeof(struct sockaddr_in);
        sd = socket(AF_INET, SOCK_DGRAM, 0);

        /* Create a datagram socket on which to receive. */

        if(sd < 0)
        {
        perror("Opening datagram socket error");
        exit(1);
        }
        /* Enable SO_REUSEADDR to allow multiple instances of this */
        /* application to receive copies of the multicast datagrams. */
        int reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
        {
        perror("Setting SO_REUSEADDR error");
        close(sd);
        exit(1);
        }
        /* Bind to the proper port number with the IP address */
        /* specified as INADDR_ANY. */
        servaddr.sin_family = AF_INET;
	    servaddr.sin_port = htons(MYPORT); /* Multicast Info service */
	
	if (inet_pton(AF_INET, "255.255.255.255", &servaddr.sin_addr) <= 0){
		fprintf(stderr,"inet_pton error for %s : 255.255.255.255 \n", strerror(errno));
    }
      if(bind(sd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
        {
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
        }

        
	char recvbuf[1024];
	len = sizeof(on);
	on=1;
	if( setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &on, len) == -1 ){
			fprintf(stderr,"SO_BROADCAST setsockopt error : %s\n", strerror(errno));
			return 1;
	}	
    int send = 0;
        
        /* Read from the socket. Oczekuje na wysyłane pakiety na ten broadcast*/
            while(1){
              
                len = sizeof(from);
                
                if(recvfrom(sd, recvbuf,1024,0,(struct sockaddr*) & from, &len)==-1){
                     perror("Błąd");
                }
                else
                {   
                    if(strcmp(recvbuf,"adres_daj")){
                       
                        }
                        else
                        {   
                            send = send + 1;
                            sendto(sd,argv[1],strlen(argv[1])+1,0,(struct sockaddr *)&servaddr,sizeof(servaddr));
                            strcpy(recvbuf,"");
                            if(send == 2){
                                printf("New Connection from %s\n",inet_ntoa(from.sin_addr));
                                send = 0;
                            }
                        }
                    
                      }
             
                }
                    
                    
            return 0;
        }
    
