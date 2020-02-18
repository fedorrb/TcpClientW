#pragma once
#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winbase.h>

//#include ".\socketcl.h"
#include <string>
#include <vector>
using namespace std;
using std::string;
using std::vector;

#pragma comment (lib, "Ws2_32.lib") //ccc
#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 131072
//#define DEFAULT_BUFLEN 65536

class Socket
{
public:
	Socket();
	Socket(string new_ip, unsigned short new_port, unsigned short attempt);
	~Socket(void);
	bool NewSocket(vector<string> & allFile, vector<string> & receivedLines);
	char errMsg[100];
	bool IsError();
	bool IsReceiveAll();
	string def_ip;
	unsigned short def_port;
	int delay_seconds;
private:
	bool receivedAllData; //ознака отримання цілого повідомлення
	bool error;
	SOCKET ConnectSocket;
	addrinfo *ptrAddrInfo;
};
