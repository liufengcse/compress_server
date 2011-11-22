
#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include "connection/SocketManager.h"
#include "ThreadManager.h"
#include "HTTPPack.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

using namespace std;

using namespace proxyserver::connection;

namespace proxyserver 
{

	const static int GS_SERVER_PORT = 3128;

	class ProxyServer
	{
	private:

		int m_serverPort;

		void initSignalHandler();

		static void show_handler(int sig);
		
	public:
		ProxyServer(int serverPort): m_serverPort(serverPort) {}


		 void startServer();
	};
}

#endif
