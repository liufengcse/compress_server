
#include "../HTTPPack.h"

#include <assert.h>

using namespace proxyserver;

int main()
{
	string 	cmd = "get",
		host = "www.test.com";
	int port = 3306;
	
	/************************ test invalid package *************/
	string pack = cmd + ' ' + host + ":3306/test.html" 
		+ " HTTP/1.1";
	HTTPPack invalidPack(pack.c_str());

	// test isValid
	assert(invalidPack.isValid() == false);
	
	/************************ test valid package ***************/
	pack = cmd + " http://" + host + ":3306/test.html"
		+ "HTP/1.1";
	HTTPPack validPack(pack.c_str());

	// test isValid
	assert(validPack.isValid() == true);

	// test getHostPort
	assert(validPack.getHostPort() == port);

	// test getHostName
	assert(validPack.getHostName().compare(host) == 0);

	// test getPack
	assert(validPack.getPack().compare(pack) == 0);

	return 0;
}
