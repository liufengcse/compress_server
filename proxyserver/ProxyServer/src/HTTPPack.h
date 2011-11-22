#ifndef HTTP_PACK_H
#define HTTP_PACK_H

#include <iostream>
#include <string>
#include <stdio.h>
#include "string.h"

using namespace std;

namespace proxyserver 
{
	class HTTPPack
	{
	private:
		const static int GS_DEFAULT_HTTP_PORT = 80;

		string m_pack;

		string m_hostName;

		int m_hostPort;

		bool m_valid;
	public:

		HTTPPack(const char *pack);

		string getPack()
		{
			return m_pack;
		}

		string getHostName() 
		{
			return m_hostName;
		}

		bool isValid()
		{
			return m_valid;
		}

		int getHostPort() 
		{
			return m_hostPort;
		}
	};
}

#endif
