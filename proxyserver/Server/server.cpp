/*****************************************************************
  server.cpp 
  enviroment: g++
  compile command: g++ server.cpp -o server -lpthread

  date:09/30/2011

  By Shangduan Wu
*****************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define QUEUE_SIZE               5
#define MAX_SIZE              1000
#define BUFFER_SIZE            100

int t_thread=0;
int message_type=1;

struct client_arg {
    int hSocket;
    sockaddr_in client;
    };

//get the extent name of the file

char* get_ext(char* filepath){

    char* p;

    if((p=strrchr(filepath,'.'))!=NULL)return p+1;

    return NULL;           
    }

//send the 404 error message to the client

void ERR404(int hSocket){

    char *msg;

    msg=(char *)"HTTP/1.1 404 Not Found";

    send(hSocket,msg,strlen(msg),0);

    }

//send the 401 error message to the client

void ERR401(int hSocket){

    char *msg;

    msg=(char *)"HTTP/1.1 401 No premission";

    send(hSocket,msg,strlen(msg),0);
  
    }

//is file existence

int fileExist(char* filename){
    FILE *fp;

    if((fp=fopen(filename,"r"))==NULL)return 0;
    fclose(fp);

    return 1;
    }

//is file security

int fileSecurity(char* filename){
    char *p;

    p=filename;
    p=p+3;
    while(*p!='\0'){
        p++;
        if(*p=='/'){
            return 0;
            }
        }
    return 1;
    }

//write the packet header to the client

int headmessage(FILE* cfp, char* extname){

    char* content=(char *)"text/plain";

    if(strcmp(extname,"html")==0||strcmp(extname,"htm")==0)
        content=(char *)"text/html";
    else if(strcmp(extname,"css")==0)
        content=(char *)"text/css";
    else if(strcmp(extname,"gif")==0)
        content=(char *)"image/gif";
    else if(strcmp(extname,"jpeg")==0||strcmp(extname,"jpg")==0)
        content=(char *)"image/jpeg";
    else if(strcmp(extname,"png")==0)
        content=(char *)"image/png";

    fprintf(cfp,"HTTP/1.1 200 OK\r\n");
    fprintf(cfp,"Content-Type: %s\r\n",content);
    return 0;
}

//send the data of the file which the client want

int sendmessage(int hSocket,char* serverfilepath){

    FILE* sfp,*cfp;
    int c;

    sfp=fopen(serverfilepath,"r");
    cfp=fdopen(hSocket,"w");

    headmessage(cfp,get_ext(serverfilepath));
    while((c=getc(sfp))!=EOF)putc(c,cfp);    
    fflush(cfp);

    return 0;
}

void getpath(char *requestline){
    int i;
    int mark;
    char * newreqline;
    i=strlen(requestline);
    newreqline=new char[MAX_SIZE];

    for(int j=1;j<i;j++){
        if((requestline[j]=='/')&&(requestline[j+1]!='/')&&(requestline[j-1]!='/')){   
            mark=j;
            break;
            }
        }    
    strcpy(newreqline, requestline+mark+1);
    strcpy(requestline, newreqline);
    }

void process_cli(int hSocket,sockaddr_in client){

    char request[MAX_SIZE], filepath[MAX_SIZE], cmd[MAX_SIZE],extname[MAX_SIZE];
    FILE *fp;

    fp=fdopen(hSocket,"r");      
    
    if(fgets(request,MAX_SIZE,fp) == NULL) {
      printf("\nCould not get socket!\n");
      exit(1);  
    }

    printf("A call request: %s\n",request);

    strcpy(filepath,"./");
    sscanf(request,"%s",cmd);
    getpath(request); 
    sscanf(request,"%s",filepath+2);
    strcpy(extname,get_ext(filepath));

    if(strcmp(cmd,"GET")==0||strcmp(cmd,"get")==0){
        if(fileExist(filepath)){
            if(fileSecurity(filepath)){
                sendmessage(hSocket,filepath);
                }
            else {
                message_type=1;
                ERR401(hSocket);
                }
            }
        else {
            message_type=1;
            ERR404(hSocket);
            }

        }

    if(close(hSocket)==-1){
        printf("\nCould not close socket\n");
        exit(1);
        }     
    }

//invoked by pthread_create

void* start_routine(void* arg){

    client_arg *info;
    info=(client_arg *)arg;

    //handle client's requirement
    process_cli(info->hSocket,info->client);

    delete (client_arg *)arg;

    t_thread=t_thread-1;

    pthread_exit(NULL);
    }

int main(int argc,char* argv[]) {
    int hServerSocket,hSocket;
    pthread_t thread;         
    client_arg *arg;            
    struct sockaddr_in server;
    struct sockaddr_in client; 
    int sin_size;
    int nHostPort;
    int thread_num;
    int message_begin;
    int opt=SO_REUSEADDR;
    char pBuffer[BUFFER_SIZE]={ };
    int n;

    if(argc<4){
	    printf("\nUsage: ./server host-port thread-num head-message (0 or 1)\n");
	    return 0;
    }
    else{
	    nHostPort=atoi(argv[1]);
      thread_num=atoi(argv[2]);
      message_begin=atoi(argv[3]);
    }

    if(message_begin!=0&&message_begin!=1){
       printf("\nInput Error for the head message type\n");
       return 0;
    }
    message_type=message_begin;

    printf("\nStarting server\n");

    printf("\nMaking socket\n");

    //make a socket

    if((hServerSocket=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("\nCould not make a socket\n");
        return 0;
    }

    setsockopt(hServerSocket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    bzero(&server,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(nHostPort);
    server.sin_addr.s_addr=htonl(INADDR_ANY);

    printf("\nBinding to port %d\n",nHostPort);

    if(bind(hServerSocket,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1){
        printf("\nCould not connect to host\n");
        return 0;
        }

    if(listen(hServerSocket,QUEUE_SIZE)==-1){
        printf("\nCould not listen\n");
        return 0;
        }

    sin_size=sizeof(struct sockaddr_in);

    while(1){
        if(t_thread<=thread_num){

        printf("\nWaiting for a connection\n");
        
        if((hSocket=accept(hServerSocket,(struct sockaddr *)&client,(socklen_t*)&sin_size))==-1){
            printf("\nCould not accept the request\n");
            return 0;
        }

	      printf("\nGot a connection\n");

        arg=new client_arg;
        arg->hSocket=hSocket;
        memcpy((void *)&arg->client,&client,sizeof(client));
     
        if(pthread_create(&thread,NULL,start_routine,(void *)arg)){
            printf("\nCould not create thread\n");
            return 0;
            }    
        else {
            t_thread=t_thread+1;
            } 
        }

        if(message_type==1) {
          n = write(hSocket,pBuffer,strlen(pBuffer)+1);
          if (n < 0) {
            printf("\nCould not write!\n");
            exit(1);  
          }
        }
        message_type=message_begin;
    }
        
    if(close(hServerSocket)==-1){
        printf("\nCould not close socket\n");
        return 0;
        }      
}
