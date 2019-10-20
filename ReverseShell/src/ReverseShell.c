#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<dirent.h>
#include<libgen.h>
#include<limits.h>
#include<pwd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/wait.h>
#include<pthread.h>
#include<netdb.h>
#include<errno.h>
#define true 1
#define false 0
#define N 4096
#define STDIN 0
#define STDOUT 1
#define STDERR 2
typedef char bool;
char program[N];
int localSockFd,remoteSockFd,n;
struct sockaddr_in localAddr, remoteAddr;
socklen_t length;

int calls(char*comando,int sockfd)
{
    /*close(STDOUT);
    dup(sockfd);
    close(STDERR);
    dup(sockfd);*/
    /*dup2(sockfd,STDOUT);
    dup2(sockfd,STDERR);*/
    mode_t mode=S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int isBackground=false;
    char*arguments[N];
    int i=0;
    int fd;
    int stds=false;
    pid_t pid;
    const char SQ[]="\'";
    const char DQ[]="\"";
    char*pathIn=NULL,*pathOut=NULL,*pathOutApp=NULL,*pathErr=NULL;
    for(char*token=strtok(comando," "); token; token=strtok(NULL," "))
    {
        if(token[0]==SQ[0])
        {
            token[strlen(token)]=' ';
            token=strtok(token,SQ);
        }
        else if(token[0]==DQ[0])
        {
            token[strlen(token)]=' ';
            token=strtok(token,DQ);
        }
        if(!strcmp(token,">"))
        {
            if(!pathOut)
            {
                if((token=strtok(NULL," "))==NULL)
                {
                    return 0;
                }
                if(token[0]=='~')
                {
                    char*path=(char*)malloc(N);
                    uid_t uid=getuid();
                    struct passwd* pwd=getpwuid(uid);
                    sprintf(path,"%s%s",pwd->pw_dir,token+1);
                    pathOut=strdup(path);
                    free(path);
                }
                else
                {
                    pathOut=strdup(token);
                }
            }
            continue;
        }
        if(!strcmp(token,">>"))
        {
            if(!pathOutApp)
            {
                if((token=strtok(NULL," "))==NULL)
                {
                    return 0;
                }
                if(token[0]=='~')
                {
                    char*path=(char*)malloc(N);
                    uid_t uid=getuid();
                    struct passwd* pwd=getpwuid(uid);
                    sprintf(path,"%s%s",pwd->pw_dir,token+1);
                    pathOut=strdup(path);
                    free(path);
                }
                else
                {
                    pathOutApp=strdup(token);
                }
            }
            continue;
        }
        if(!strcmp(token,"<"))
        {
            if(!pathIn)
            {
                if((token=strtok(NULL," "))==NULL)
                {
                    return 0;
                }
                if(token[0]=='~')
                {
                    char*path=(char*)malloc(N);
                    uid_t uid=getuid();
                    struct passwd* pwd=getpwuid(uid);
                    sprintf(path,"%s%s",pwd->pw_dir,token+1);
                    pathOut=strdup(path);
                    free(path);
                }
                else
                {
                    pathIn=strdup(token);
                }
            }
            continue;
        }
        if(!strcmp(token,"2>"))
        {
            if(!pathErr)
            {
                if((token=strtok(NULL," "))==NULL)
                {
                    return 0;
                }
                if(token[0]=='~')
                {
                    char*path=(char*)malloc(N);
                    uid_t uid=getuid();
                    struct passwd* pwd=getpwuid(uid);
                    sprintf(path,"%s%s",pwd->pw_dir,token+1);
                    pathOut=strdup(path);
                    free(path);
                }
                else
                {
                    pathErr=strdup(token);
                }
            }
            continue;
        }
        if((i>=1) && (!strcmp(token,"&")))
        {
            isBackground=true;
            continue;
        }
        arguments[i]=strdup(token);
        i++;
    }
    arguments[i]=NULL;
    //printf("%s\n%ld\n",comando,strlen(comando));
    if(!strcmp(arguments[0],program))
    {
        fprintf(stderr,"%s: this program it's already run\n",program);
        return 0;
    }
    if(!strcmp(arguments[0],"cd"))
    {
        if(!arguments[1])
        {
            uid_t uid=getuid();
            struct passwd* pwd=getpwuid(uid);
            chdir(pwd->pw_dir);
        }
        else
        {
            if(arguments[1][0]=='~')
            {
                char*path=(char*)malloc(N);
                uid_t uid=getuid();
                struct passwd* pwd=getpwuid(uid);
                sprintf(path,"%s%s",pwd->pw_dir,arguments[1]+1);
                if((chdir(path))==-1)
                {
                    dup2(sockfd,STDERR);
                    fprintf(stderr,"%s: \"%s\": ",program,path);
                    perror("");
                    dup2(STDERR,sockfd);
                }
                free(path);
            }
            else if((chdir(arguments[1]))==-1)
            {
                dup2(sockfd,STDERR);
                fprintf(stderr,"%s: \"%s\": Not a directory\n", arguments[0], arguments[1]);
                dup2(STDERR,sockfd);
            }
        }
        return 0;
    }
    if((pid=fork())==-1)
    {
        perror("can't create new process");
        return -1;
    }
    if(!pid)
    {
        dup2(sockfd,STDIN);
        dup2(sockfd,STDOUT);
        dup2(sockfd,STDERR);
        if(pathIn)
        {
            if((access(pathIn,F_OK))==-1)
            {
                fprintf(stderr,"%s: ",program);
                perror("");
                exit(2);
            }
            if((access(pathIn,R_OK))==-1)
            {
                fprintf(stderr,"%s: ",program);
                perror("");
                exit(2);
            }
            int fd;
            if((fd=open(pathIn,O_RDONLY))==-1)
            {
                fprintf(stderr,"%s: ",program);
                perror("");
                exit(2);
            }
            close(STDIN);
            dup(fd);
        }
        if(pathOut)
        {
            if((fd=open(pathOut, O_WRONLY | O_CREAT | O_TRUNC,mode))==-1)
            {
                fprintf(stderr,"%s: ",program);
                perror("");
                exit(2);
            }
            close(STDOUT);
            dup(fd);
        }
        if(pathOutApp)
        {
            if((fd=open(pathOutApp, O_RDWR | O_APPEND | O_CREAT,mode))==-1)
            {
                fprintf(stderr,"%s: ",program);
                perror("");
                exit(2);
            }
            close(STDOUT);
            dup(fd);
        }
        if(pathErr)
        {
            if((fd=open(pathErr,O_RDWR | O_APPEND | O_CREAT,mode))==-1)
            {
                fprintf(stderr,"%s: ",program);
                perror("");
                exit(2);
            }
            close(STDERR);
            dup(fd);
        }
        printf("\"%s\"\n",arguments[0]);
        if((execvp(arguments[0],arguments))==-1)
        {
            fprintf(stderr,"%s: \"%s\": ",program,arguments[0]);
            perror("");
            exit(2);
        }
        //close(fd);
        exit(0);
    }
    else
    {
        if(!isBackground)
        {
            wait(NULL);
        }
    }
    if(pathIn)
    {
        //printf("ciao\n");
        free(pathIn);
    }
    if(pathOut)
    {
        //printf("free Output\n");
        free(pathOut);
    }
    if(pathOutApp)
    {
        //printf("ciao\n");
        free(pathOutApp);
    }
    if(pathErr)
    {
        //printf("free Error\n");
        free(pathErr);
    }
    i=0;
    while(arguments[i])
    {
        //printf("%s ",arguments[i]);
        //printf("%d\n",i);
        free(arguments[i]);
        i++;
    }
    return 0;
}

void*connection(void*dummy)
{
    int newRemoteSockFd=remoteSockFd;
    struct sockaddr_in newRemoteAddr=remoteAddr;
    socklen_t newLength=length;
    char*buffer=(char*)malloc(N);
    char cwd[PATH_MAX];
    ssize_t size;
    printf("%s: Client from: %s:%d connected, it's address is %u bytes long\n",program,inet_ntoa(newRemoteAddr.sin_addr),ntohs(newRemoteAddr.sin_port),newLength);
    char*draw=(char*)malloc(N);
    sprintf(draw," ____  ____  _  _  ____  ____  ___  ____      ___  _   _  ____  __    __   \n(  _ \\( ___)( \\/ )( ___)(  _ \\/ __)( ___)___ / __)( )_( )( ___)(  )  (  )  \n )   / )__)  \\  /  )__)  )   /\\__ \\ )__)(___)\\__ \\ ) _ (  )__)  )(__  )(__ \n(_)\\_)(____)  \\/  (____)(_)\\_)(___/(____)    (___/(_) (_)(____)(____)(____)\n\n\n");
    write(newRemoteSockFd,draw,strlen(draw));
    while(true)
    {
        if(!getcwd(cwd,PATH_MAX))
        {
            fprintf(stderr,"%s: ",program);
            perror("");
            return NULL;
        }
        char revsh[PATH_MAX];
        sprintf(revsh,"bash-sh:%s reverse-shell$ ", basename(cwd));
        write(newRemoteSockFd,revsh,strlen(revsh));
        do{
            if((size=read(newRemoteSockFd,buffer,N))==-1)
            {
                fprintf(stderr,"%s: ",program);
                perror("");
                free(buffer);
                return NULL;
            }
        }while(N==size);
        if(size==0)
        {
            break;
        }
        else if(size==1)
        {
            continue;
        }
        if(buffer[strlen(buffer)-1]=='\n')
        {
            buffer[strlen(buffer)-1]='\0';
        }
        printf("\n\t%s %lu\n",buffer,strlen(buffer));
        if((!strcmp(buffer,"quit")) || (!strcmp(buffer,"q")) || (!strcmp(buffer,"QUIT")) || (!strcmp(buffer,"Q")))
        {
            break;
        }
        if((calls(buffer,newRemoteSockFd))==-1)
        {
            fprintf(stderr,"%s: ",program);
            perror("");
            free(buffer);
            return NULL;
        }
        memset((char*)buffer,0,sizeof(buffer));
        //printf("ciao\n");
    }
    close(newRemoteSockFd);
    if(buffer)
    {
        free(buffer);
    }
    return NULL;
}

int server(int port)
{
    //struct hostent*host;
    if((localSockFd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        fprintf(stderr,"%s: ",program);
        perror("");
        return -1;
    }
    memset((char*)&localAddr,0,sizeof(localAddr));
    localAddr.sin_family=AF_INET;
    localAddr.sin_addr.s_addr=inet_addr("0.0.0.0");
    localAddr.sin_port=htons(port);
    //char*ip=inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));
    printf("%s: Wait for connection on port %d…\n",program,port);
    if((bind(localSockFd,(struct sockaddr*)&localAddr,sizeof(localAddr)))==-1)
    {
        fprintf(stderr,"%s: ",program);
        perror("");
        return -1;
    }
    listen(localSockFd,1000);
    while(true)
    {
        pthread_t connect;
        length=sizeof(remoteAddr);
        if((remoteSockFd=accept(localSockFd,(struct sockaddr*)&remoteAddr,&length))==-1)
        {
            fprintf(stderr,"%s: ",program);
            perror("");
            return -1;
        }
        pthread_create(&connect,NULL,(void*)&connection,NULL);
    }
    close(localSockFd);
    return 0;
}

void help()
{
    printf("usage: %s <port>\n",program);
    printf("This program simulate a Reverse-Shell, for remote command from another shell in the same LAN.\n");
}

int main(int argc,char**argv)
{
    extern int errno;
    strcpy(program,argv[0]);
    if(argc>=3)
    {
        printf("usage: %s <port>\n",program);
    }
    else if((argc>=2) && (!strcmp(argv[1],"--help")))
    {
        help();
    }
    else if(argc==1)
    {
        char*buffer=(char*)malloc(N);
        size_t length;
        while(true)
        {
            printf("%s: Port ",program);
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
        int port=atoi(buffer);
        if(buffer)
        {
            free(buffer);
        }
        if((server(port))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    else if(argc==2)
    {
        int port;
        port=atoi(argv[1]);
        if((server(port))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}
