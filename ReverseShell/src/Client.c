#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/wait.h>
#include<pthread.h>
#include<errno.h>
#define true 1
#define false 0
#define N 4096
typedef char bool;
typedef struct
{
	int sockfd;
	struct sockaddr_in remoteAddr;
	socklen_t length;
} SocketInfo;
char program[255];

void*sending(void*data)
{
	SocketInfo*info=data;
	char*buffer=(char*)malloc(N);
	//size_t length;
	while(true)
	{
		fgets(buffer,N,stdin);
		//length=strlen(buffer);
		if((write(info->sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("");
			free(buffer);
			return NULL;
		}
        if((!strcmp(buffer,"quit")) || (!strcmp(buffer,"q")) || (!strcmp(buffer,"QUIT")) || (!strcmp(buffer,"Q")))
        {
            break;
        }
	}
	if(buffer)
	{
		free(buffer);
	}
	return NULL;
}

void*recive(void*data)
{
	SocketInfo*info=data;
	char*buffer=(char*)malloc(N);
	ssize_t size;
	while(true)
	{
		do{
			if((size=read(info->sockfd,buffer,N))==-1)
			{
				fprintf(stderr,"%s: ",program);
				perror("");
				free(buffer);
				return NULL;
			}
			if((write(1,buffer,size))==-1)
			{
				fprintf(stderr,"%s: ",program);
				perror("");
				free(buffer);
				return NULL;
			}
		}while(size==N);
	}
	if(buffer)
	{
		free(buffer);
	}
	return NULL;
}

int client(char*inetAddr,int port)
{
	SocketInfo*info=(SocketInfo*)malloc(sizeof(SocketInfo));
	if((info->sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("");
		free(info);
		return -1;
	}
	memset((char*)&info->remoteAddr,0,sizeof(info->remoteAddr));
	info->remoteAddr.sin_family=AF_INET;
	info->remoteAddr.sin_addr.s_addr=inet_addr(inetAddr);
	info->remoteAddr.sin_port=htons(port);
	connect(info->sockfd,(struct sockaddr*)&info->remoteAddr,sizeof(info->remoteAddr));
	pthread_t th1,th2;
	pthread_create(&th1,NULL,(void*)&sending,info);
	pthread_create(&th2,NULL,(void*)&recive,info);
	pthread_join(th1,NULL);
    pthread_cancel(th2);
	close(info->sockfd);
	if(info)
	{
		free(info);
	}
	return 0;
}

int main(int argc,char**argv)
{
	extern int errno;
	strcpy(program,argv[0]);
	char*inetAddr;
	int port;
	if(argc>=4)
	{
		printf("usage: %s <IP address> <port>\n",program);
		exit(EXIT_SUCCESS);
	}
	else if(argc==1)
	{
		char*buffer=(char*)malloc(15);
		size_t length;
		while(true)
		{
			printf("%s: IP address",program);
			fgets(buffer,N,stdin);
			if((length=strlen(buffer))==1)
			{
				continue;
			}
			else if(buffer[length-1]=='\n')
			{
				buffer[length-1]='\0';
			}
			break;
		}
		inetAddr=strdup(buffer);
		while(true)
		{
			printf("%s: port",program);
			fgets(buffer,N,stdin);
			if((length=strlen(buffer))==1)
			{
				continue;
			}
			else if(buffer[length-1]=='\n')
			{
				buffer[length-1]='\0';
			}
			break;
		}
		port=atoi(buffer);
		if(buffer)
		{
			free(buffer);
		}
	}
	else if(argc==2)
	{
		inetAddr=strdup(argv[1]);
		char*buffer=(char*)malloc(N);
		size_t length;
		while(true)
		{
			printf("%s: port",program);
			fgets(buffer,N,stdin);
			if((length=strlen(buffer))==1)
			{
				continue;
			}
			else if(buffer[length-1]=='\n')
			{
				buffer[length-1]='\0';
			}
			break;
		}
		port=atoi(buffer);
		if(buffer)
		{
			free(buffer);
		}
	}
	else if(argc==3)
	{
		inetAddr=strdup(argv[1]);
		port=atoi(argv[2]);
	}
	if((client(inetAddr,port))==-1)
	{
		free(inetAddr);
		exit(EXIT_FAILURE);
	}
	if(inetAddr)
	{
		free(inetAddr);
	}
	exit(EXIT_SUCCESS);
}
