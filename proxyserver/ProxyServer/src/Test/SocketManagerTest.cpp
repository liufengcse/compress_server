
#include <assert.h>

#include "../connection/SocketManager.h"

using namespace proxyserver::connection;

const static int GS_SERVER_PORT = 6688;
const static int GS_PACK_LEN = 256;

int main(int argc, char *argv[]) 
{
	const string testStr = "Test String.";			// test string
	int procID;

	if ((procID = fork()) > 0)
	{// server	
		SocketManager serSockMan;			// server socket manager

		int serSock = serSockMan.			// server socket
			makeServerSocket(GS_SERVER_PORT);

		int fd = accept(serSock,NULL,NULL);		// file ID
		if (fd == -1)
			perror("Server Accept Failure");

		write(fd,testStr.c_str(),			// write the message
			testStr.length() * sizeof(testStr[0])
		);
		close(fd);
	}
	else if (procID == 0)
	{// client
		sleep(1);					// wait for the server

		SocketManager cliSockMan;			// client socket manager
		
		int cliSock = cliSockMan.			// client socket
			makeClientSocket("localhost",
				GS_SERVER_PORT);

		char read_buf[GS_PACK_LEN];			// read buffer

		// read the message
		int readBytes = read(cliSock,read_buf,GS_PACK_LEN);	
		read_buf[readBytes] = '\0';

		string cmpStr(read_buf);

		// compare message
		assert(testStr.compare(cmpStr) == 0);
	}
	else 
	{
		perror("Fork Error");
	}

	return 0;
}
