#include "stdafx.h"
#include ".\socketcl.h"
#include <ctime>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
using std::string;

Socket::Socket()
{
	errMsg[0] = '\0';
	ptrAddrInfo = NULL;
	receivedAllData = false;
	error = false;
	ConnectSocket = INVALID_SOCKET;
	def_ip = "10.0.8.101";
	def_port = 1861;
	delay_seconds = 1;
}

Socket::Socket(string new_ip, unsigned short new_port, unsigned short attempt)
{
	errMsg[0] = '\0';
	ptrAddrInfo = NULL;
	receivedAllData = false;
	error = false;
	ConnectSocket = INVALID_SOCKET;
	def_ip = new_ip;
	def_port = new_port;
	delay_seconds = attempt;
}

Socket::~Socket(void)
{
	freeaddrinfo(ptrAddrInfo);
	WSACleanup();
}

bool Socket::IsError() {
	return error;
}

bool Socket::IsReceiveAll() {
	return receivedAllData;
}

bool Socket::NewSocket(vector<string> & allFile, vector<string> & receivedLines) {
	bool result = false;
	WSADATA wsaData;
	int iResult = 0;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct sockaddr_in clientService;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN-2;

	//----------------------
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR) {
		strcpy(errMsg, "WSAStartup failed");
		error = true;
		return 1;
	}

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		strcpy(errMsg, "Error at socket: ");
		strcat(errMsg, strerror(WSAGetLastError()));
		error = true;
		return 1;
	}

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( def_ip.c_str() );
	clientService.sin_port = htons( def_port );

	// Connect to server.
	iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
	if ( iResult == SOCKET_ERROR) {
		closesocket (ConnectSocket);
		strcpy(errMsg, "Unable to connect to server");
		error = true;
		return 1;
	}

	// Send an initial buffer
	iResult = send( ConnectSocket, allFile[0].c_str(), allFile[0].size(), 0 );
	if (iResult == SOCKET_ERROR) {
		strcpy(errMsg, "Send failed");
		closesocket(ConnectSocket);
		error = true;
		return 1;
	}

	// shutdown the connection since no more data will be sent
	/*
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		//printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	*/

	receivedLines.clear();
	clock_t delay = delay_seconds * CLOCKS_PER_SEC;
	clock_t start = clock();
	clock_t finish = clock();
	recvbuf[0] = '\0';
	int reeceivedBytes = 0;
	// Receive until the peer closes the connection
	do {
		try {
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		} catch (exception& e) {
			iResult = -1;
		}
		if(iResult > recvbuflen)
			iResult = recvbuflen;
		if(iResult > 0) {
			reeceivedBytes += iResult;
			recvbuf[iResult] = '\0';
			receivedLines.push_back(recvbuf);
			if(recvbuf[iResult-1] == '\1' && recvbuf[iResult-2] == '\n' && recvbuf[iResult-3] == '\r') {
				receivedAllData = true;
				break;
			}
			else {
				//Sleep(100);
			}
		}
		else if ( iResult == 0 ) {
			if(!receivedAllData) {
				finish = clock();
				if(finish - start < delay) {
					iResult = 1;
					Sleep(1000);
					continue;
				}
			}
			strcpy(errMsg, "Connection closed");
			error = true;
		}
		else {
			strcpy(errMsg, "Recv failed with error: ");
			strcat(errMsg, strerror(WSAGetLastError()));
			error = true;
			if(!receivedAllData) {
				finish = clock();
				if(finish - start < delay) {
					iResult = 1;
					Sleep(1000);
					continue;
				}
			}
		}
	} while( iResult > 0 );

	// cleanup
	iResult = shutdown(ConnectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR) {
		strcpy(errMsg, "Cocket shutdown failed: ");
		strcat(errMsg, strerror(WSAGetLastError()));
		closesocket(ConnectSocket);
		return 1;
	}
	closesocket(ConnectSocket);
	return result;
}