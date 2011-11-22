
#include "ProxyServer.h" 

int main(int argc, char *argv[]) {
	using namespace proxyserver;

	int serverPort=GS_SERVER_PORT;

	// get the host port
	if(argc!=2)
	{
	      printf("Usage: ./ProxyServer proxy-port\n");
	      return 0;
	}
        else 
        {
	      serverPort=atoi(argv[1]);
        }

	ProxyServer proxyServer(serverPort);

	proxyServer.startServer();				// start proxy server

	return 0;
}

namespace proxyserver {
	/***************** starServer ******************************/
	void ProxyServer::startServer() 
	{
		int serSock, cliSock; 				// server socket
		SocketManager serSockMan; 			// server socket manager

		// initialize to handle singnals
		initSignalHandler();

		printf("******** Start Http Proxy Server on Port %d ********\n",
			m_serverPort);
		
		// create server socket
		serSock = serSockMan.
			makeServerSocket(m_serverPort); 			
		if(serSock == -1)
		{
			fprintf(stderr,"Setup Server Socket Failed\n");
			exit(1);
		}

		// handle client connection
		while (true) 
		{
			cliSock = accept(serSock, NULL, NULL);
			
			if (cliSock!=-1) 
			{
				ThreadManager::createProxyThread(cliSock);
			}
			else
			{
				printf("\nConnect Client Error\n");
			}
		}
	} // starServer


	/************************ InitSignalHandler ***********************/
	void ProxyServer::initSignalHandler() {
            struct sigaction act,oldact;

            printf("process id is %d\n",getpid());
            act.sa_handler=show_handler;
            sigemptyset(&act.sa_mask);
            act.sa_flags=0;

            if(sigaction(SIGHUP,&act,&oldact)<0)goto loop;
            if(sigaction(SIGINT,&act,&oldact)<0)goto loop;
            if(sigaction(SIGQUIT,&act,&oldact)<0)goto loop;
            if(sigaction(SIGILL,&act,&oldact)<0)goto loop;
            if(sigaction(SIGTRAP,&act,&oldact)<0)goto loop;
            if(sigaction(SIGABRT,&act,&oldact)<0)goto loop;
            if(sigaction(SIGBUS,&act,&oldact)<0)goto loop;
            if(sigaction(SIGFPE,&act,&oldact)<0)goto loop;
            if(sigaction(SIGUSR1,&act,&oldact)<0)goto loop;   
            if(sigaction(SIGSEGV,&act,&oldact)<0)goto loop;  
            if(sigaction(SIGUSR2,&act,&oldact)<0)goto loop;  
            if(sigaction(SIGPIPE,&act,&oldact)<0)goto loop; 
            if(sigaction(SIGALRM,&act,&oldact)<0)goto loop;  
            if(sigaction(SIGTERM,&act,&oldact)<0)goto loop;   
            if(sigaction(SIGXCPU,&act,&oldact)<0)goto loop;   
            if(sigaction(SIGXFSZ,&act,&oldact)<0)goto loop;   
            if(sigaction(SIGVTALRM,&act,&oldact)<0)goto loop;   
            if(sigaction(SIGPROF,&act,&oldact)<0){
loop:       printf("Install Signal Error\n");
            exit(1);
            }   
	} // InitSignalHandler

	/************************ show_handler ***************************/
	void ProxyServer::show_handler(int sig) {
            printf("Got Signal %d\n",sig);
            exit(0);
	} // show_handler

} // namespce proxyserver
