/*****************************************************************
  client.cpp 
  enviroment: g++
  compile command: g++ client.cpp -o web_client -lpthread

  date:11/04/2011
*****************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <string>

using namespace std; 

#define USERAGENT "HTMLGET 1.1"
#define MAX_FILE_NUMBER 1000
#define MAX_FILE_NAME 100
#define TRY_TIME 10

int t_thread;
double total_size_send=0.0;
double total_size_recv=0.0;
double total_time=0.0;
double thread_avg_time=0.0; 

char file_name[MAX_FILE_NUMBER][MAX_FILE_NAME];

struct input_arg{
    char *host;
    int port;
    int acess_num;
	int file_num;
	char *proxyname;
    int proxyport;
};

int fileExist(char* filename){
    FILE *fp;

    if((fp=fopen(filename,"r"))==NULL)return 0;
    fclose(fp);

    return 1;
    }
 
int fileSecurity(char* filename){
    char *p;

    p=filename;
    if(*p=='/'){
        return 0;
        }
    while(*p!='\0'){
        p++;
        if(*p=='/'){
            return 0;
            }
        }
    return 1;
    }

int create_socket(){
    int hSocket;

    if((hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
        printf("\nCould not create socket\n");
        exit(1);
        }

    return hSocket;
    }

char *get_hostname(char *host){
    struct hostent *hent;
    int ip_length=15;
    char *ip=(char *)malloc(ip_length+1);

    memset(ip,0,ip_length+1);
    if((hent=gethostbyname(host))==NULL){
        printf("\nCould not get IP\n");
        exit(1);
        }
    if(inet_ntop(AF_INET,(void *)hent->h_addr_list[0],ip,ip_length)==NULL){
        printf("\nCould not resolve host\n");
        exit(1);
        }
    return ip;
    }

char *build_query(char *host,int port,char *page){
    char *query;
    char *getpage=page;
    char *tpl=(char *)"GET http://%s:%d/%s\r\nHost:%s\r\nUser-Agent: %s\r\n";
  
    if(getpage[0]=='/'){
        getpage=getpage+1;
        fprintf(stderr,"Removing leading \"/\",converting %s to %s\n",page,getpage);
        }
    query=(char *)malloc(2*strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
    sprintf(query,tpl,host,port,getpage,host,USERAGENT);

    return query;
    }

int oneclient(char * host,int port,char * proxyname,int proxyport,char* page){
    struct sockaddr_in *remote;
    struct sockaddr_in *proxy;
    int sock;
    int tmpres;
    int proxy_tmpres;
    int sent;
    char *ip;
    char *proxy_ip;
    char *get;
    char buf[BUFSIZ+1];
    int htmlstart=0;
    char *htmlcontent;

    //the filename is related to thread id and acess#, it will be used storage the fetched file

    sock=create_socket();
    proxy_ip=get_hostname(proxyname);
    fprintf(stderr,"proxy_IP is %s\n",proxy_ip);
    proxy=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
    proxy->sin_family=AF_INET;
    proxy_tmpres=inet_pton(AF_INET,proxy_ip,(void *)(&(proxy->sin_addr.s_addr)));

    if(proxy_tmpres<0){
        printf("\nCould not set client->sin_addr.s_addr\n");
        exit(1);
        }
    else if(proxy_tmpres==0){
        fprintf(stderr, "\n%s is not a valid host name\n", ip);
        exit(1);
        }
    proxy->sin_port=htons(proxyport);

    if(connect(sock,(struct sockaddr *)proxy,sizeof(struct sockaddr))<0){
        printf("\nCould not connect the server\n");
        exit(1);
        }
    get=build_query(host,port,page);
 
    //Send the query to the server
    sent=0;
    while(sent<strlen(get)){
        tmpres=send(sock,get+sent,strlen(get)-sent,0);
        if(tmpres==-1){
            printf("\nCould not send query\n");
            exit(1);
            }
        sent+=tmpres;
        }
    total_size_send=total_size_send+sent;

    //now it is time to receive the page
    memset(buf,0,sizeof(buf));
    while((tmpres=recv(sock,buf,BUFSIZ,0))>0){
        total_size_recv=total_size_recv+tmpres;
        if(htmlstart==0){
            htmlcontent = buf;

            if(htmlcontent!=NULL){
                htmlstart=1;
                htmlcontent+=4;
                }
            }
		else {
            htmlcontent = buf;
            }

        if(htmlstart){
            fprintf(stdout,"%s\n",htmlcontent);
            }

        memset(buf,0,tmpres);
        }
  
    if(tmpres<0)printf("\nError receiving data\n");

    free(get);
    free(remote);
    free(proxy);
    free(ip);
    free(proxy_ip);

    if(close(sock)==-1){
        printf("\nCould not close socket\n");
        return 0;
        }
    return 0;
    }

void * start_routine(void* arg){
    input_arg *info;
    info=(input_arg *)arg;
    char* page;
	int file_num;
    int num;
	int i,k,m;
    clock_t begin;
    clock_t end;

    //handle client's requirement
    begin=clock();
    for(i=1;i<=info->acess_num;i++){
	    file_num=info->file_num;
        m=0;
loop:   k=(int)((float)file_num*rand()/RAND_MAX);
        page=file_name[k];

	    if(fileExist(page)){
            if(fileSecurity(page)){
                }
            else {
                printf("\nNo permission for the file\n");
                m=m+1;
                if(m<TRY_TIME){
                    goto loop;
                    }
                else {
                    printf("\nExit for No permission\n");
                    exit(1);
                    }
                }
            }
        else {
            printf("\nFile not exist\n");
            m=m+1;
            if(m<TRY_TIME){
                goto loop;
                }
            else {
                printf("\nExit for No files\n");
                exit(1);
                }
            }

        oneclient(info->host,info->port,info->proxyname,info->proxyport,page);
        }

    end=clock();
    
    thread_avg_time+=(double)(end-begin)/CLOCKS_PER_SEC;
    }

int main(int argc, char* argv[]){
   input_arg *thread_args;
   int i;
   char *HOST;
   char *PROXYNAME;
   int PORT;
   int PROXYPORT;
   int ACCESS_NUM;
   int NUM_THREADS;
   int file_num;
   clock_t begin,end;
   int n;

   if(argc!=9){
       printf("\nUsage: ./web_client host-name host-port thread-num access-num file-num -proxy proxy-name proxy-port\n");
       return 0;
       }
   else {
       thread_args=new input_arg;

       HOST=argv[1];
       thread_args->host=HOST;
       PORT=atoi(argv[2]);
       thread_args->port=PORT;
       NUM_THREADS=atoi(argv[3]);
       ACCESS_NUM=atoi(argv[4]);
       thread_args->acess_num=ACCESS_NUM;
       file_num=atoi(argv[5]);
       thread_args->file_num=file_num;

       PROXYNAME=argv[7];
       PROXYPORT=atoi(argv[8]);

       thread_args->proxyport=PROXYPORT;
       thread_args->proxyname=PROXYNAME;
       }

    printf("\nEnter name of files:\n");

    for(i=0;i<file_num;i++){
        n = scanf("%s",file_name[i]);
        getchar();
    }

    pthread_t thread[NUM_THREADS];
 
    begin=clock();
    //create all threads
    for(i=0;i<NUM_THREADS;i++){
        if(pthread_create(&thread[i],NULL,start_routine,(void *)thread_args)){
	    printf("\nCould not create thread\n");
	    return 0;
	    }
        else {
            t_thread=t_thread+1;
	    }
        }
 
    //wait for all threads to complete
    for(i=0;i<NUM_THREADS;i++) {
	if(pthread_join(thread[i],NULL)){
	    printf("\nCould not join thread\n");
	    return 0;
            }
        }

    end=clock();

    delete thread_args;

    total_time=total_time+(double)(end-begin)/CLOCKS_PER_SEC;
    printf("Sent data per Second:%fMB/s\n",total_size_send/1024/1024/total_time);
    printf("Receive Data per Second: %fMB/s\n",total_size_recv/1024/1024/total_time);
    printf("Total time: %fs\n",total_time);
    printf("Request per second: %f\n",(ACCESS_NUM*NUM_THREADS)/total_time);
    printf("Time pre Request: %fs\n",thread_avg_time/(ACCESS_NUM*NUM_THREADS));
}
