#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <pthread.h>
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"
#define SNMP     161
#define SNMPTRAP 162
#define MAXLINE 1024
char ipaddr[16][16];
int ports[16];
int t = 0;

struct thread_args
	{
		int sockfd;
        struct sockaddr_in cliaddr;
	};

int setup_sock(int sockfd, struct sockaddr_in servaddr){
    
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) { 
    perror("socket creation failed"); 
    exit(EXIT_FAILURE); 
    }
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    return sockfd;

}

struct sockaddr_in setup_sockaddr_in(struct sockaddr_in servaddr, int PORT){
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_family = AF_INET; 

    return servaddr;

}

void* trap(void* a){
   
    struct sockaddr_in cliaddr_trap = setup_sockaddr_in(cliaddr_trap, SNMPTRAP);
    int sockfdtrap = setup_sock(sockfdtrap, cliaddr_trap);
    char buffer2[MAXLINE]; 
    char trap[MAXLINE] = "trap";
    int len, x;
    x = 0;
    len = sizeof(cliaddr_trap);
      while(1){
        recvfrom(sockfdtrap, (char *)buffer2, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr_trap, &len); 
        if(strncmp(buffer2,trap,4) == 0){
            printf("TRAP\n");
            char databuf2[1024];
		    char data3[1024];
        	char str1[] = " ";
			strncpy(databuf2,buffer2 + 7 ,strlen(buffer2) - 7);
            databuf2[strlen(buffer2) - 7] = 0;
            int x = strcspn(databuf2, str1);
            char info[x];
            strncpy(info,databuf2,x);
            info[x] = 0;
            strncpy(data3,databuf2 + x + 1,strlen(databuf2) - x - 1);
            data3[strlen(databuf2) - x - 1 ] = '\0';
            if(buffer2[5] == 'B'){
                x = atoi(info);
				    printf("Stan baterii w laptopie %s to ", data3);
				if(x < 30){
				    printf(RED "%s " RESET,info);
				}else if(x>30 && x<70){
				    printf(YEL "%s " RESET,info);
				}else{
				    printf(GRN "%s " RESET,info);
				}
				printf("procent.\n");
                }
            else if(buffer2[5] == 'T'){
                int x = atoi(info);
                x = x/1000;
                printf("Temperatura procesora w urządzeniu %s to ", data3);
                if(x < 40){
                    printf(GRN "%d " RESET,x);
                }else if(x>40 && x<60){
                    printf(YEL "%d " RESET,x);
                }else{
                    printf(RED "%d " RESET,x);
                }
                    printf("stopni.\n");
                }
            }
        memset(buffer2,0,MAXLINE);
        }
}


void* polling(void *a){
    struct thread_args *arg = (struct thread_args *)a;
    int sockfd = arg ->sockfd;
    struct sockaddr_in cliaddr = arg ->cliaddr;
    char *hello = "get-bulk-request";
    while(1){
        if(t > 0){
            for(int i = 0; i < t; i++){
                cliaddr.sin_addr.s_addr = inet_addr(ipaddr[i]);
                cliaddr.sin_port = htons(ports[i]);
                sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr)); 
                sleep(0.1);
            }
            cliaddr.sin_addr.s_addr = INADDR_ANY;
            cliaddr.sin_port = SNMP;
        }
        sleep(10);
    }

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

void tab(FILE *plik,char* nazwa){
	int size = strlen(nazwa);
	if(size > 24){
		strncpy(nazwa,nazwa,21);
		char* dots[] = {"..."};
		char out[25];
		strncpy(out,nazwa,20);
		strcat(out,"...");
		fprintf(plik, "%s\t", out);
	}else{
		fprintf(plik, "%s",nazwa);
		for(size; size < 24; size = size+8){
			fprintf(plik,"\t");
		}
	}
}


int main() { 

    int sockfd;
    struct sockaddr_in cliaddr = setup_sockaddr_in(cliaddr, SNMP);
    sockfd = setup_sock(sockfd, cliaddr);
    pthread_t snmp_trap;
    pthread_t snmp_poll;
    time_t rawtime;

  			struct tm * timeinfo;
    struct thread_args args;
    args.cliaddr = cliaddr;
    args.sockfd = sockfd;
    pthread_create(&snmp_trap,NULL, trap, NULL);
    pthread_create(&snmp_poll,NULL, polling, (void *)&args);
    char buffer[MAXLINE];
    char buffer1[MAXLINE];
    int len;
    len = sizeof(cliaddr);
    char new_agent[MAXLINE] = "new-agent";
    char get_reqest[MAXLINE] = "get-request";
    char get_response[MAXLINE] = "get-response";
    char ans_agnet[MAXLINE] = "ok";
    FILE *outfile_bat;
    if(exists("batery_stat.txt") == 0){
        outfile_bat = fopen("batery_stat.txt","w");
        fprintf(outfile_bat, "TIME\t\tIP.ADDR\t\tNAME\t\t\tBATTERY STATUS\n");
        fclose(outfile_bat);
    }
    FILE *outfile_temp;
    if(exists("temp_stat.txt") == 0){
        outfile_temp = fopen("temp_stat.txt","w");
        fprintf(outfile_temp, "TIME\t\tIP.ADDR\t\tNAME\t\t\tTEMP STATUS\n");
        fclose(outfile_temp);
    }
    while(1){
        recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
        if(strcmp(buffer,new_agent) == 0){
            sendto(sockfd, (const char *)ans_agnet, strlen(ans_agnet), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
            inet_ntop(AF_INET, &(cliaddr.sin_addr), ipaddr[t], 16);
            ports[t] = ntohs(cliaddr.sin_port);
            t++;
        }
        else if(strncmp(buffer,get_response,12) == 0){
            								time (&rawtime);
								timeinfo = localtime (&rawtime);
								strftime (buffer1,80,"%X",timeinfo);
                		char databuf2[1024];
					    char data3[1024];
                    	char str1[] = " ";
						strncpy(databuf2,buffer + 15 ,strlen(buffer) - 15);
                        databuf2[strlen(buffer) - 15] = 0;
                        int x = strcspn(databuf2, str1);
                        char info[x];
                        strncpy(info,databuf2,x);
                        info[x] = 0;
                        strncpy(data3,databuf2 + x + 1,strlen(databuf2) - x - 1);
                        data3[strlen(databuf2) - x - 1 ] = '\0';
                        if(buffer[13] == 'B'){
                            x = atoi(info);
							printf("Stan baterii w laptopie %s to ", data3);
							if(x < 30){
								printf(RED "%s " RESET,info);
							}else if(x>30 && x<70){
								printf(YEL "%s " RESET,info);
							}else{
								printf(GRN "%s " RESET,info);
							}
							printf("procent.\n");
                            FILE *plik;
									plik = fopen("batery_stat.txt","a+");
									fprintf(plik,"%s\t%s\t",buffer1,inet_ntoa(cliaddr.sin_addr));
									tab(plik,data3);
									fprintf(plik,"%s\n",info);
									fclose(plik);
                        }
                        else if(buffer[13] == 'T'){
                            int x = atoi(info);
							x = x/1000;
							printf("Temperatura procesora w urządzeniu %s to ", data3);
							if(x < 40){
							    printf(GRN "%d " RESET,x);
							}else if(x>39 && x<60){
							    printf(YEL "%d " RESET,x);
							}else{
							    printf(RED "%d " RESET,x);
							}
							    printf("stopni.\n");
                                FILE *plik;
									plik = fopen("temp_stat.txt","a+");
									fprintf(plik,"%s\t%s\t",buffer1,inet_ntoa(cliaddr.sin_addr));
									tab(plik,data3);
									fprintf(plik,"%d\n",x);
									fclose(plik);
                        }
                    }
        memset(buffer,0,MAXLINE);
    }

}