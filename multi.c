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
#include <sys/wait.h>
#include <time.h>    
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"
#define MYPORT_INFO 4321
struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
int len;
struct sockaddr_in from;
char databuf[1024];


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

int main(int argc, char* argv[])
{	
	char ip[1024];
	if (argc < 2) {
		char ip_def[1024] = "226.1.1.1";						
		strcpy(ip,ip_def);
	}
	else {
		strcpy(ip,argv[1]);
	}		
			
			char* arg[] = {"pidof ./serv"};
			pidof(arg[0]);
	  		time_t rawtime;
  			struct tm * timeinfo;
			char buffer [80];
			char* args[] = {"./serv_br"};
			pid_t madka = getpid();
			fork();
			if(getppid() == madka){
				execl(args[0],args[0],ip,NULL);
			}else{
			int len = sizeof(struct sockaddr_in);
			sd = socket(AF_INET, SOCK_DGRAM, 0);
		/* Create a datagram socket on which to receive. */

			if (sd < 0)
			{
				perror("Opening datagram socket error");
				exit(1);
			}
			/* Enable SO_REUSEADDR to allow multiple instances of this */
			/* application to receive copies of the multicast datagrams. */
			
				int reuse = 1;
				if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0)
				{
					perror("Setting SO_REUSEADDR error");
					close(sd);
					exit(1);
				}
						/* Bind to the proper port number with the IP address */
						/* specified as INADDR_ANY. */
					memset((char*)&localSock, 0, sizeof(localSock));
					localSock.sin_family = AF_INET;
					localSock.sin_port = htons(MYPORT_INFO);
					localSock.sin_addr.s_addr = INADDR_ANY;

					/* Join the multicast group 226.1.1.1 on the local */
					/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
					/* called for each ciekawe czy ktos to czyta local interface over which the multicast */
					/* datagrams are eleanor rigby. */
					group.imr_multiaddr.s_addr = inet_addr(ip);
					if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(group)) < 0)
					{
						perror("Adding multicast group error");
						close(sd);
						exit(1);
					}
					if (bind(sd, (struct sockaddr*) & localSock, sizeof(localSock)))
					{
						perror("Binding datagram socket error");
						close(sd);
						exit(1);
					}
						/* Read from the socket. Oczekuje na wysyłane pakiety na ten multicast*/
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
					int i = 0;
					while (1) {
						strcpy(databuf,"                                                                                    ");
				
						len = sizeof(from);
						if ((recvfrom(sd, databuf, 1024, 0, (struct sockaddr*) & from, &len) < 0)) {
							perror("Reading datagram message error");
							close(sd);
							exit(1);
						}
						else {	
							char databuf2[1024];
							char data3[1024];
							if(databuf[0] == 'B' || databuf[0] == 'T' ){
								time (&rawtime);
								timeinfo = localtime (&rawtime);
								strftime (buffer,80,"%X",timeinfo);
								char str1[] = " ";
								strncpy(databuf2,databuf + 2 ,strlen(databuf) - 2);
								databuf2[strlen(databuf) - 2] = 0;
								int x = strcspn(databuf2, str1);
								char info[x];
								strncpy(info,databuf2,x);
								info[x] = 0;
								printf("%s INFO #%i \n", buffer, i);
								i++;
								strncpy(data3,databuf2 + x + 1,strlen(databuf2) - x - 1);	
								data3[strlen(databuf2)-x-1] = 0;								
								if(databuf[0] == 'B'){
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
									printf("Adres IP urządzenia to : %s\n", inet_ntoa(from.sin_addr));
									FILE *plik;
									plik = fopen("batery_stat.txt","a+");
									fprintf(plik,"%s\t%s\t",buffer,inet_ntoa(from.sin_addr));
									tab(plik,data3);
									fprintf(plik,"%s\n",info);
									fclose(plik);
									
								}else{
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
									printf("Adres IP urządzenia to : %s\n", inet_ntoa(from.sin_addr));
									FILE *plik;
									plik = fopen("temp_stat.txt","a+");
									fprintf(plik,"%s\t%s\t",buffer,inet_ntoa(from.sin_addr));
									tab(plik,data3);
									fprintf(plik,"%d\n",x);
									fclose(plik);
								}

								}								
						}
					}
				return 0;
			
		}
	}
