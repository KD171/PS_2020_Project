#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <syslog.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include "deamon.h"
#define SNMP     161
#define SNMPTRAP 162
#define MAXLINE 1024

struct thread_args
	{
		int sockfd;
		char *tempfile;
		char *batfile;
        struct sockaddr_in servaddr;
	};


void info_sent(int sd2, char* databuf, int datalen,struct sockaddr_in servaddr ){
    
	if(sendto(sd2, databuf,datalen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		{
			syslog(LOG_ERR,"Sending datagram message error: %s\n", strerror(errno));
		}

	else{
			syslog (LOG_INFO,"Sending datagram message...OK");
	}
}

int stan(const char *fname){
	FILE *infile;
	infile = fopen(fname,"r");
	char S[10];													
	int x;														
	fscanf(infile,"%s", S);
	x = atoi(S);
	fclose(infile);
	return x;
	}

int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int setup_sock(int sockfd){
    
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) { 
    perror("socket creation failed"); 
    exit(EXIT_FAILURE); 
    }
    struct timeval tv;
	tv.tv_sec = 2;  /* 2 Secs Timeout */
    tv.tv_usec = 0;
	if( setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1 ){
    	fprintf(stderr,"SO_RCVTIMEO setsockopt error : %s\n", strerror(errno));
	    exit(1);
	}

    return sockfd;

}

struct sockaddr_in setup_sockaddr_in(struct sockaddr_in servaddr, int PORT, char argv[MAXLINE]){
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_addr.s_addr = inet_addr(argv); 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_family = AF_INET; 

    return servaddr;

}

char * hostname(){
    char *str = (char *) malloc(sizeof(char) * 1024);
    FILE *infile;
	infile = fopen("/etc/hostname","r");											
	fscanf(infile,"%s", str);
	fclose(infile);
    return str;
}

char * path_bat(){
    char bateria[] = "/sys/class/power_supply/BAT0/capacity";
	for(int x = 0; x < 9; x++){
		if(exists(bateria) == 1){
	    	break;
		}else{
			bateria[27] = x + '0';
		}
	}
    char *output = (char *) malloc(sizeof(char) * 1024);
    strcpy(output,bateria);
    return output;

}


char * path_temp(){
    char temperatura[] = "/sys/class/hwmon/hwmon0/temp1_input";
	for(int x = 0; x < 9; x++){
		if(exists(temperatura) == 1){
	    	break;
		}else{
            temperatura[22] = x + '0';
		}
	}
    char *output = (char *) malloc(sizeof(char) *( strlen(temperatura)+1));
    strcpy(output,temperatura);
    return output;
}


void* trap(void* a){

	char *hostn = hostname();
	struct thread_args *arg = (struct thread_args *)a;	
	int sockfd = arg -> sockfd;
	char bateria[MAXLINE];
	strcpy(bateria,arg -> batfile);
	char temperatura[MAXLINE];
	strcpy(temperatura,arg -> tempfile);
	char databuf[1024];
	char space[] = " ";
	int len;
	daemon_init("./agent", LOG_USER, 1000, sockfd);
	while (1)
    {
			if(exists(bateria) == 1){
			sprintf(databuf,"%d",stan(bateria));
			if(atoi(databuf) < 30){
				char spaceB[1024] = "trap B ";
				strcat(spaceB,databuf);
				strcat(spaceB,space);
				strcat(spaceB,hostn);
				len = sizeof(spaceB);
				info_sent(sockfd, spaceB, len, arg ->servaddr);
			}
		}
		if(exists(temperatura) == 1){
			sprintf(databuf,"%d",stan(temperatura));
			if(atoi(databuf) > 70000){
				char spaceT[1024] = "trap T ";
				strcat(spaceT,databuf);
				strcat(spaceT,space);
				strcat(spaceT,hostn);
				len = sizeof(spaceT);
				info_sent(sockfd, spaceT, len, arg ->servaddr);
			}
		}
		sleep(2);
    }
}

int main(int argc, char* argv[]) { 

    int sockfdtrap, sockfd;
    pthread_t snmptrap_t;
    struct sockaddr_in servaddr, servaddr161; 
    sockfdtrap = setup_sock(sockfdtrap);
    sockfd = setup_sock(sockfd);
    servaddr = setup_sockaddr_in(servaddr,SNMPTRAP,argv[1]);
    servaddr161 = setup_sockaddr_in(servaddr161,SNMP,argv[1]);
    char *hostn = hostname();
    char *temperatura = path_temp();
    char *bateria = path_bat();
    struct thread_args trap_arg;
	trap_arg.sockfd = sockfdtrap;
	trap_arg.tempfile = temperatura;
	trap_arg.batfile = bateria;
    trap_arg.servaddr = servaddr;
    pthread_create(&snmptrap_t,NULL, trap, (void *)&trap_arg);
	daemon_init(argv[0], LOG_USER, 1000, sockfd);
	syslog (LOG_NOTICE, "Program started by User %d", getuid ());
	syslog (LOG_INFO,"Send Info to server"); 
    int n, len;
    char databuf[MAXLINE], databuf2[MAXLINE], buffer2[MAXLINE], buffer[MAXLINE];
    char *new_agent = "new-agent"; 
	char get_reqest[MAXLINE] = "get-bulk-request";
    char space[] = " ";
    len = sizeof(servaddr161);
    sendto(sockfd, (const char *)new_agent, strlen(new_agent), MSG_CONFIRM, (const struct sockaddr *) &servaddr161, sizeof(servaddr161)); 

    while(1){
		recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr161, &len); 
		if(strcmp(buffer,"ok") == 0){
			break;
		}
        memset(buffer,0,MAXLINE);
		sendto(sockfd, (const char *)new_agent, strlen(new_agent), MSG_CONFIRM, (const struct sockaddr *) &servaddr161, sizeof(servaddr161)); 
	}
    while (1)
    {
        n = recvfrom(sockfd, (char *)buffer2, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr161, &len); 
        buffer2[n] = '\0'; 
        if(strcmp(buffer2,get_reqest) == 0){
			int x = 0;
            	if(exists(bateria) == 1){
				sprintf(databuf,"%d",stan(bateria));
				char spaceB[1024] = "get-response B ";
				strcat(spaceB,databuf);
				strcat(spaceB,space);
				strcat(spaceB,hostn);
				len = sizeof(spaceB);
				info_sent(sockfd, spaceB, len, servaddr161);
				x++;
				sleep(0.1);
			}
			if(exists(temperatura) == 1){
				sprintf(databuf,"%d",stan(temperatura));
				char spaceT[1024] = "get-response T ";
				strcat(spaceT,databuf);
				strcat(spaceT,space);
				strcat(spaceT,hostn);
				len = sizeof(spaceT);
				info_sent(sockfd, spaceT, len, servaddr161);
				x++;
				sleep(0.1);
			}
			memset(buffer2,0,MAXLINE);

        }
  
    }
    close(sockfd);
    close(sockfdtrap); 
    return 0; 
} 
