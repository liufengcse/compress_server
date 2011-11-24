
#include "ThreadManager.h"

namespace proxyserver {
	/************************ initailize static member **********/
	const string ThreadManager::MS_CANNT_GET_MSG 
		= "Can't connect to server";

	/************************ createProxyThread *****************/
	void ThreadManager::createProxyThread(int cliSocket)
	{
		pthread_t proxyThread;				// thread
		pthread_attr_t detachedAttr;			// thread attribute
		int *cliSocketP;				// client socket pointer
		
		// initalize the thread attribute to be detached
		pthread_attr_init(&detachedAttr);
		if(pthread_attr_setdetachstate(&detachedAttr, 
			PTHREAD_CREATE_DETACHED) != 0)
		{
			perror("Set Thread Attribute Failed");
			return;
		}
		
		// copy the clisocket
		cliSocketP = new int;
		*cliSocketP = cliSocket;

		// create proxy thread
		if(pthread_create(&proxyThread, &detachedAttr, 
			processRequest, cliSocketP) != 0)
		{
			perror("Create Thread Error");
			return;
		}
	}

	/************************ processRequest *******************/
	void * ThreadManager::processRequest(void *cliSocketP)
	{
		// copy the client socket
		int cliSock = *static_cast<int *>(cliSocketP);
		delete static_cast<int *>(cliSocketP);

		char pack[BUFSIZ];				// package to be the cache between client 
								// and  server(here server is mean the server 
								// which provide the resources that client 
								// ask for, below is the same meaning)
		int bytesRead;
		SocketManager serSockMan;			// server socket manager
		int serSock;					// server socket
        bool isImage;
        char *imgBuf;
        imgBuf = new char[IMGSIZ];
		
		if ((bytesRead = read(cliSock, pack, BUFSIZ))) 
		{						
			// Get the HTTP command and arguments
			if (bytesRead < 0 && errno != EINTR)
			{
				perror("Read Client Error");
				close(cliSock);
				return NULL;
			}
			
			// set end to the pack
			pack[bytesRead] = '\0';

			// make up the HTTP package
			HTTPPack firstHttpPack(pack);
			
			// Is the package valid?
			if (!firstHttpPack.isValid())
			{
				close(cliSock);
				return NULL;
			}

			// connect to server
			serSock = serSockMan.
				makeClientSocket(firstHttpPack.getHostName().c_str(), 
					firstHttpPack.getHostPort());
			if (serSock <= 0 && errno != EINTR) 
			{
				perror("Connect Server Error");
				myWrite(cliSock, MS_CANNT_GET_MSG.c_str(),
					MS_CANNT_GET_MSG.length());
				close(cliSock);
				return NULL;
			}
			
			// send the request to server
			if(myWrite(serSock, firstHttpPack.getPack().c_str(), 
				firstHttpPack.getPack().length() * sizeof(char)) == -1) 
			{
				perror("Write to Server Failure");
				close(cliSock);
				return NULL;
			}
/*Modified here */

            //whether it is image file

            if(!(isImage = ImageFile(pack))){
            //transfer between two sockets            
			    transBetSerAndCli(serSock,cliSock);
            }else{
               cout <<"the target is an image" << endl;
               DownImg(serSock, imgBuf);
  //             CompressImg();
               SendImg(cliSock, imgBuf);
            }

		}
			
		close(cliSock);
		return NULL;
	} // processRequest

/************************ tansBetSerAndCli *****************/
	void DownImg(int serSock, char *buf)
	{
		
//		char buf [BUFSIZ];				// read buf
		fd_set rdfdset;					// read file set
		struct timeval timeout;				// time out value
		int selVal;					// select value
        int i ;
        int iolen;
		
		// wait for client and server for 1 seconds
		timeout.tv_sec = 1;				
		timeout.tv_usec = 0;
        i = 0;
		
		
			// Select for writable on either of our two sockets 
			FD_ZERO(&rdfdset);
			FD_SET(serSock, &rdfdset);
			if ((selVal = select(FD_SETSIZE,&rdfdset,NULL,NULL,&timeout)) < 0) 
			{
				perror("Select Failed");
				return;
			}
            cout << "selVal  "<< selVal << endl;
			if(selVal == 0 || selVal == -1)
				return;

			// is the client ready for writing?
			if (FD_ISSET(serSock, &rdfdset))
			{
				if ((iolen = read(serSock, buf, sizeof(buf))) <= 0)
					return;
			}
           
           
			
		
	} // tranBetSerAndCli


/************************ tansBetSerAndCli *****************/
	void SendImg(int cliSock, char *buf)
	{
		
//		char buf [BUFSIZ];				// read buf
		fd_set rdfdset;					// read file set
		struct timeval timeout;				// time out value
		int selVal;					// select value
        int i ;
		
		// wait for client and server for 1 seconds
		timeout.tv_sec = 1;				
		timeout.tv_usec = 0;
        i = 0;
		
		
			// Select for writable on either of our two sockets 
			FD_ZERO(&rdfdset);
			FD_SET(cliSock, &rdfdset);
			if ((selVal = select(FD_SETSIZE,NULL, &rdfdset, NULL, &timeout)) < 0) 
			{
				perror("Select Failed");
				return;
			}
            cout << "selVal  "<< selVal << endl;
			if(selVal == 0 || selVal == -1)
				return;

			// is the client ready for writing?
			if (FD_ISSET(cliSock, &rdfdset))
			{
				// copy to client -- blocking semantics
				myWrite(cliSock, buf, sizeof(buf)); 	 
			}
           
			
		
	} // tranBetSerAndCli

 	/************************ tansBetSerAndCli *****************/
	void ThreadManager::transBetSerAndCli(int serSock, int cliSock)
	{
		int iolen;					// read length
		char buf [BUFSIZ];				// read buf
		fd_set rdfdset;					// read file set
		struct timeval timeout;				// time out value
		int selVal;					// select value
		
		// wait for client and server for 1 seconds
		timeout.tv_sec = 1;				
		timeout.tv_usec = 0;
		
		while (1)
		{
			// Select for readability on either of our two sockets 
			FD_ZERO(&rdfdset);
			FD_SET(cliSock, &rdfdset);
			FD_SET(serSock, &rdfdset);
			if ((selVal = select(FD_SETSIZE, &rdfdset, NULL, NULL, &timeout)) < 0) 
			{
				perror("Select Failed");
				break;
			}

			if(selVal == 0)
				break;

			// is the client sending data? 
			if (FD_ISSET(cliSock, &rdfdset))
			{
				// zero length means the client disconnected 
				if ((iolen = read(cliSock, buf, sizeof(buf))) <= 0)
					break; 			

				// copy to host -- blocking semantics
				myWrite(serSock, buf, iolen); 	 
			}

			// is the host sending data?
			if (FD_ISSET(serSock, &rdfdset))
			{
				// zero length means the host disconnected
				if ((iolen = read(serSock, buf, sizeof(buf))) <= 0)
					break; 			 

				// copy to client -- blocking semantics
				myWrite(cliSock, buf, iolen); 	 
			}
		}
	} // tranBetSerAndCli

	int myWrite(int fd, const void *buf, int count)
	{
		int bytesWrite;					// bytes write
		char *ptr = (char *)buf;			// write buffer pointer

		while((bytesWrite = write(fd,ptr,count))) 
		{
			// a critcal error happen
			if((bytesWrite == -1) && (errno != EINTR))
				break;
			// all bytes has been writed
			else if(bytesWrite == count)
				break;	
			// part of the bytes has been writed, contiue to write
			else if(bytesWrite > 0) 
			{
				ptr += bytesWrite;
				count -= bytesWrite;
			}
		}

		if(bytesWrite == -1)
			// write failure
			return -1;						
		else
			// write success
			return 0;				
	} // myWrite

//determine whether it is an image

    bool ImageFile(char * request){
        bool isImage = false;
        int length;
        int i;

        length = strlen(request);

        for(i = 4; i < length; i++){
            if(request[i-3] == '.' && request[i-2] == 'j' && request[i-1] == 'p' && request[i] == 'g' ){
                isImage = true;
                break;
            }
            
            if(request[i-4] == '.' && request[i-3] == 'j' && request[i-2] == 'p' && request[i-1] == 'e' && request[i] == 'g' ){
                isImage = true;
                break;
            }
        }
    
        return isImage;
    }
/*
    void DownImg(int socket, char *buf){
        int size;
        if((size = read(socket, buf, IMGSIZ)) <= 0){
            perror("Cannot download img");
        }
        cout << "down load image size is :"<< size << endl;
    }

    void SendImg(int socket, char *buf){
        int size;
        if((size = write(socket, buf, IMGSIZ)) <= 0){
            perror("cannot send img");
        }
        cout << "sent image size is :"<< size<< endl;
    }
*/
} // namespace proxyserver
