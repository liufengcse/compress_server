
#ifndef THREAD_MANAGER_H
#define TGREAD_MANAGER_H

#include "HTTPPack.h"
#include "connection/SocketManager.h"

#include <pthread.h>
#include <sys/time.h>

#include <errno.h>

#include <iostream>
#include <string>
#include <stdio.h>

#define IMGSIZ 8192

using namespace std;

using namespace proxyserver::connection;

namespace proxyserver {

	class ThreadManager
	{
	private:

		const static string MS_CANNT_GET_MSG;


		static void * processRequest(void *cliSockP);

		static void transBetSerAndCli(int serSock, int cliSock); 
	public:

		static void createProxyThread(int cliSock);
	
	};

	
	int myWrite(int fd, const void *buf, int count);
    bool ImageFile(char *);
    void DownImg(int socket, char *buf);
    void SendImg(int socket, char *buf);
    

}// namespace proxyserver

#endif
