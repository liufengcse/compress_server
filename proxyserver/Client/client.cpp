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
#define MAX_FILE_NUMBER 10
#define MAX_FILE_NAME 50

struct ARG{
    char * host;
    int port;
    int acess_num;
    int file_num;
    int proxyport;
    char *proxyname;
    int arg_num;
};

int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host, int port, char *page);
int oneclient(char * host, int port , char * proxyname, int proxyport, char * page, int acess);
void * start_routine(void * arg);
void initiate_filelist(int);

double total_size_send=0.0;
double total_size_recv=0.0;
double total_time=0.0;
double thread_avg_time=0.0; 


char file_list[MAX_FILE_NUMBER][MAX_FILE_NAME];

int main(int argc, char** argv){

   int rc, i;
   time_t begin, end;
   char *HOST;
   char *PROXYNAME;
   int PORT;
   int PROXYPORT;
   int ACESS_NUM;
   int file_num;
   int NUM_THREADS;

   ARG thread_args;

   if(argc != 9 && argc != 6){
        printf("usage: ./web_client hostname port thread_num acess_num file_name -proxy proxyname proxyport\n");
        printf("    - hostname:port: server address and port\n");
        printf("    - thread_num: \n");
        printf("    - access_num: \n");
        printf("    - file_num: number of files are going to be fetched\n");
        printf("    - proxyname:proxyport: proxy address and port\n");

        exit(1);
    }

    HOST = argv[1];
    PORT = atoi(argv[2]);
    NUM_THREADS = atoi(argv[3]);
    ACESS_NUM = atoi(argv[4]);
    file_num = atoi(argv[5]);
    if(file_num > MAX_FILE_NUMBER - 1) {
      printf("Err: Beyond the max file number!\n");
      exit(1);

    }
    thread_args.port = PORT;
    thread_args.host = HOST;
    thread_args.acess_num = ACESS_NUM;
    thread_args.file_num = file_num;
 
    if(argc == 9){
        PROXYNAME = argv[7];
        PROXYPORT = atoi(argv[8]);
        pthread_t threads[NUM_THREADS];
        thread_args.proxyport = PROXYPORT;
        thread_args.proxyname = PROXYNAME;
        thread_args.arg_num = 5;
    }
    else {thread_args.arg_num = 3;}

    initiate_filelist(file_num);

    pthread_t threads[NUM_THREADS];

    begin = clock();

   /* create all threads */
   for (i=0; i<NUM_THREADS; ++i) {
//      thread_args[i] = i;
      printf("In main: creating thread %d\n", i);
      rc = pthread_create(&threads[i], NULL, start_routine, (void *)&thread_args);
      assert(0 == rc);
   }

   /* wait for all threads to complete */
   for (i=0; i<NUM_THREADS; ++i) {
      rc = pthread_join(threads[i], NULL);
      assert(0 == rc);
   }
    end = clock();

    total_time=total_time+(double)(end-begin)/CLOCKS_PER_SEC;
    printf("Sent data MB/s:%f\n", total_size_send/1024/1024/total_time);
    printf("Received data MB: %d\n", total_size_recv/1024/1024);
    printf("Received data MB/s: %f\n", total_size_recv/1024/1024/total_time);
    printf("Total time: %f\n", total_time);
    printf("Average time each thread: %f\n", thread_avg_time);
    printf("Request per second: %f\n", ACESS_NUM*NUM_THREADS/total_time);
 
   exit(EXIT_SUCCESS);
}

void initiate_filelist(int file_num){
    int i, n;

    for(i=0;i<file_num;i++){
        printf("\nEnter name of files:\n");
        n = scanf("%s",file_list[i]);
        getchar();
        printf("\nYou have entered file name: %s\n", file_list[i]);
    }

/*
    strcpy(file_list[0], "accep.html");
    strcpy(file_list[1], "bind.html");
    strcpy(file_list[2], "connect.html");
    strcpy(file_list[3], "listen.html");
    strcpy(file_list[4], "recv.html");
    strcpy(file_list[5], "send.html");
    strcpy(file_list[6], "sendmsg.html");
    strcpy(file_list[7], "shutdown.html");
    strcpy(file_list[8], "socket.html");
    strcpy(file_list[9], "socketpair.html");
*/
}

void * start_routine(void * arg){
    ARG *info;
    info = (ARG *)arg;
    char* mypage;
    time_t begin;
    time_t end;
    int num;
    //handle client's requirement
    begin = clock();
    
    num = (int) ((float)((float)rand()/RAND_MAX)*(info->file_num-1));
    for(int i = 0; i < info->acess_num; i++){
        cout << num << endl;
        mypage = file_list[num];
        cout << mypage << endl;
       
        oneclient(info->host, info->port, info->proxyname, info->proxyport, mypage, i);

    }
    end = clock();
    
    thread_avg_time += (double) (end - begin)/CLOCKS_PER_SEC;
    pthread_exit(NULL);
}
 
int oneclient(char * host, int port , char * proxyname, int proxyport, char* page, int acess)
{
  struct sockaddr_in *remote;
  struct sockaddr_in *proxy;
  int sock;
  int tmpres;
  int proxy_tmpres;
  char *ip;
  char *proxy_ip;
  char *get;
  char buf[BUFSIZ+1];
  char filename[50];
  FILE * output;
  size_t sz;
  string s1(page);
  sz = s1.size();
  pthread_t id = pthread_self();

  /*the filename is related to thread id and acess#, it will be used storage the fetched file*/
  if (s1.find("html") != string::npos) {
    s1.resize(sz-5);
    sprintf(filename, "%s_%u_%d.html", s1.c_str(), (unsigned int)id, acess);
  } else if (s1.find("j") != string::npos) {
    s1.resize(sz-4);
    sprintf(filename, "%s_%u_%d.jpg", s1.c_str(), (unsigned int)id, acess);
  } 
  
  output = fopen(filename, "w");
  if(output == NULL){
    perror("cannot open file");
    exit(1);
  }

  sock = create_tcp_socket();
  proxy_ip = get_ip(proxyname);
  fprintf(stderr, "proxy_IP is %s\n", proxy_ip);
  proxy = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  proxy->sin_family = AF_INET;
  proxy_tmpres = inet_pton(AF_INET, proxy_ip, (void *)(&(proxy->sin_addr.s_addr)));
  if( proxy_tmpres < 0)  
  {
    perror("Can't set proxy->sin_addr.s_addr");
    exit(1);
  }else if(proxy_tmpres == 0)
  {
    fprintf(stderr, "%s is not a valid proxy_IP address\n", proxy_ip);
    exit(1);
  }
  proxy->sin_port = htons(proxyport);
/*
  ip = get_ip(host);
  fprintf(stderr, "IP is %s\n", ip);
  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  remote->sin_family = AF_INET;
  tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
  if( tmpres < 0)  
  {
    perror("Can't set remote->sin_addr.s_addr");
    exit(1);
  }else if(tmpres == 0)
  {
    fprintf(stderr, "%s is not a valid IP address\n", ip);
    exit(1);
  }
  remote->sin_port = htons(port);
 */
  if(connect(sock, (struct sockaddr *)proxy, sizeof(struct sockaddr)) < 0){
    perror("Could not connect");
    exit(1);
  }
  get = build_get_query(host, port, page);
  fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);

 
  //Send the query to the server
  int sent = 0;
  while(sent < strlen(get))
  {
    tmpres = send(sock, get+sent, strlen(get)-sent, 0);
    if(tmpres == -1){
      perror("Can't send query");
      exit(1);
    }
    sent += tmpres;
  }
   total_size_send += sent;
  //now it is time to receive the page
  memset(buf, 0, sizeof(buf));
  int htmlstart = 0;
  char * htmlcontent;

  while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){
    printf("Recv: %d\n", tmpres);
    fflush(stdout);
    total_size_recv = total_size_recv+tmpres;

//    if(htmlstart == 0)
//    {
      /* Under certain conditions this will not work.
      * If the \r\n\r\n part is splitted into two messages
      * it will fail to detect the beginning of HTML content
      */

//     htmlcontent = strstr(buf, "\r\n\r\n");
//      htmlcontent = buf;

//      if(htmlcontent != NULL) {
//        htmlstart = 1;
//        htmlcontent += 4;
//      }
//    } else {
      htmlcontent = buf;
//    }

//    if(htmlstart){
      //fprintf(output, htmlcontent);
      printf(htmlcontent);
//    }
 
    memset(buf, 0, tmpres);
  }
  
  if(tmpres < 0)
  {
    perror("Error receiving data");
  }

  free(get);
  free(remote);
  free(proxy);
  free(ip);
  free(proxy_ip);
  close(sock);
  return 0;
}
 

 
int create_tcp_socket()
{
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    perror("Can't create TCP socket");
    exit(1);
  }
  return sock;
}
 
 
char *get_ip(char *host)
{
  struct hostent *hent;
  int iplen = 15; //XXX.XXX.XXX.XXX
  char *ip = (char *)malloc(iplen+1);
  memset(ip, 0, iplen+1);
  if((hent = gethostbyname(host)) == NULL)
  {
    herror("Can't get IP");
    exit(1);
  }
  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
  {
    perror("Can't resolve host");
    exit(1);
  }
  return ip;
}
 
char *build_get_query(char *host, int port, char *page)
{
  char *query;
  char *getpage = page;
//  char *tpl = "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\n\r\n";
    char *tpl = "GET http://%s:%d/%s\r\nHost:%s\r\nUser-Agent: %s\r\n";
  if(getpage[0] == '/'){
    getpage = getpage + 1;
    fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
  }
  // -5 is to consider the %s %s %s in tpl and the ending \0
  query = (char *)malloc(2*strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
  sprintf(query, tpl, host, port, getpage, host, USERAGENT);
  return query;
}
