#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <syslog.h>
#include <fcntl.h>  
#include <signal.h>
#define	MAXFD	64
#define MYPORT 9009 //for broadcast
#define MYPORT_INFO 4321 //dla info_stat
struct in_addr localInterface;
struct sockaddr_in groupSock,Reciver;
int sd,on,olen,addr_len,sd2;
char space[] = " ";

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

void info_sent(int sd2, char* databuf, int datalen){
	if(sendto(sd2, databuf, datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
		{
			syslog(LOG_ERR,"Sending datagram message error: %s\n", strerror(errno));
		}

	else{
			syslog (LOG_INFO,"Sending datagram message...OK");
	}
}

int daemon_init(const char *pname, int facility, uid_t uid, int socket)
{
	int		i, p;
	pid_t	pid;

	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		exit(0);			/* parent terminates */

	/* child 1 continues... */

	if (setsid() < 0)			/* become session leader */
		return (-1);

	signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		exit(0);			/* child 1 terminates */

	/* child 2 continues... */

	chdir("/tmp");				/* change working directory  or chroot()*/
//	chroot("/tmp");

	/* close off file descriptors */
	for (i = 0; i < MAXFD; i++){
		if(socket != i )
			close(i);
	}

	/* redirect stdin, stdout, and stderr to /dev/null */
	p= open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	openlog(pname, LOG_PID, facility);
	
        syslog(LOG_ERR," STDIN =   %i\n", p);
	setuid(uid); /* change user */
	
	return (0);				/* success */
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

int main(int argc, char* argv[]){
	
	char* arg[] = {"pidof ./cliv"};
	pidof(arg[0]);
	int t;
	if (argc < 2) {
		t = 5;
	}
	else {
		t = atoi(argv[1]);

		if(t < 1){
			t = 1;
		}
	}	
	char databuf[1024];
	char databuf2[1024];
	char recvbuf[1024];
	FILE *infile;						
	infile = fopen("/etc/hostname","r");
	char S[1024];											
	fscanf(infile,"%s", databuf2);
	fclose(infile);
	printf("Nazwa hosta to %s\n",databuf2); 
	char bateria[]= "/sys/class/power_supply/BAT0/capacity";
	char temperatura[] = "/sys/class/hwmon/hwmon0/temp1_input";
	for(int x = 0; x < 6; x++){
		if(exists(temperatura) == 1){
		printf("Wykryto czujnik temperatury CPU\n");
		break;
		}else{
			temperatura[22] = x + '0';
		}
	}
	for(int x = 0; x < 9; x++){
		if(exists(bateria) == 1){
		printf("Wykryto czujnik stanu baterii\n");
		break;
		}else{
			bateria[27] = x + '0';
		}
	}

	int datalen = sizeof(databuf);
	memset((char*)&groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_port = htons(MYPORT_INFO);
	int reuse = 1;
	memset((char*)&Reciver, 0, sizeof(Reciver));
	Reciver.sin_addr.s_addr = inet_addr("255.255.255.255");
	Reciver.sin_family = AF_INET;
	Reciver.sin_port   = htons(MYPORT);	/*  server port*/
	addr_len =  sizeof(Reciver);
	if ( (sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr,"AF_INET socket error : %s\n", strerror(errno));
		return 2;
		}
	if ( (sd2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr,"AF_INET socket error : %s\n", strerror(errno));
		return 2;
		}
	if (bind(sd, (struct sockaddr *) &Reciver, sizeof(Reciver)) < 0){
        perror("bind() failed");
		close(sd);
   		exit(1);
		}
	if (bind(sd2, (struct sockaddr *) &groupSock, sizeof(groupSock)) < 0){
        perror("bind() failed");
		close(sd2);
   		exit(1);
		}
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
    	perror("Setting SO_REUSEADDR error");
    	close(sd);
   		exit(1);
    	}
	if(setsockopt(sd2, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
    	perror("Setting SO_REUSEADDR error");
    	close(sd2);
    	exit(1);
    	}
	if(setsockopt(sd2, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0){
		perror("Setting local interface error\n");
  		exit(1);
		}
	olen = sizeof(on);
	on=1;
	if( setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &on, olen) == -1 ){
		fprintf(stderr,"SO_BROADCAST setsockopt error : %s\n", strerror(errno));
		return 3;
		}	
	char empty[11];
	strcpy(empty,"adres_daj");
	int emptylen = sizeof(empty);
	if(sendto(sd, empty, emptylen, 0, (struct sockaddr *)&Reciver, sizeof(Reciver)) < 0)
		{
			perror("Sending datagram message error");
		}
		else
		{
			printf("Wysyłam żadanie o adres multicastu...\nCzekam na odpowiedź...\n");
		}
	strcpy(recvbuf,"adres_daj");
	int lengt = sizeof(recvbuf);
	while(1){
		if( (( recvfrom(sd, recvbuf, 1024, 0, NULL,0) )) <0 )
	{
		printf("Reading datagram error \n");
	}
		else{
		if(strcmp("adres_daj",recvbuf)){
				printf("Adres odebrany z serwera to : %s\n",recvbuf);
				printf("Wysyłam update stanu co %d sekund\n",t);
				break;
			}else{
				sleep(2);
				sendto(sd, empty, emptylen,0, (struct sockaddr *)&Reciver, sizeof(Reciver));
				
			}

		}
	}
	groupSock.sin_addr.s_addr = inet_addr(recvbuf);
	close(sd);
	sleep(0.2);
	daemon_init(argv[0], LOG_USER, 1000, sd2);
	syslog (LOG_NOTICE, "Program started by User %d", getuid ());
	syslog (LOG_INFO,"Send Info to server");


		while(1){
			if(exists(bateria) == 1){
				sprintf(databuf,"%d",stan(bateria));
				char spaceB[1024] = "B ";
				strcat(spaceB,databuf);
				strcat(spaceB,space);
				strcat(spaceB,databuf2);
				datalen = sizeof(spaceB);
				info_sent(sd2, spaceB, datalen);
			}
			if(exists(temperatura) == 1){
				sprintf(databuf,"%d",stan(temperatura));
				char spaceT[1024] = "T ";
				strcat(spaceT,databuf);
				strcat(spaceT,space);
				strcat(spaceT,databuf2);
				datalen = sizeof(spaceT);
				info_sent(sd2, spaceT, datalen);
			}
			sleep(t);
		}
	return 0;
}

