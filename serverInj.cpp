#define WIN32_LEAN_AND_MEAN

#define IPCLIENT "172.16.24.45"

#include "windows.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "stdlib.h"
#include "stdio.h"
#include "iostream"

#define DEBUGPROG

using namespace std;

	  // Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
#ifndef DEBUGPROG
	char *sendbuf = argv[2];
#else
	char *sendbuf = "fichier.exe";
#endif
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
#ifndef DEBUGPROG
	if (argc != 3) {
		cout << "usage: " << argv[0] <<  " server-name" <<  " filename" <<+ endl;  //***
		return;
	}
#endif
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed with error: " << iResult << endl;
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
#ifdef DEBUGPROG
	iResult = getaddrinfo(IPCLIENT, DEFAULT_PORT, &hints, &result);
#else
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &amp; hints, &amp; result);
#endif
	if (iResult != 0) {
		cout << "getaddrinfo failed with error: " << iResult << endl;
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			cout << "socket failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		return;
	}

	// Send the whole content of sendbuf
	cout << "length sendbuf = " << (int)strlen(sendbuf) << endl;
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
	if (iResult == SOCKET_ERROR) {
		cout << "send failed with error: " << WSAGetLastError() << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	cout << "Bytes Sent: " << iResult << endl;



	bool done;
	int i, cumul;
	int taille;;  //***

	cumul = 0; //***
	while (cumul < sizeof(int) - 1) {   //*** reçoit la taille du fichier (un entier...)
		iResult = recv(ConnectSocket,
			((char *)(&taille)) + cumul,
			sizeof(taille),
			0);
		cumul += iResult;
	}

	done = false;
	cumul = 0;

	FILE * recv_file;

	fopen_s(&recv_file, sendbuf, "ab");  //*** ouverture du fichier avant de recevoir les informations

	while (cumul < taille - 1) {
		iResult = recv(ConnectSocket,
			&recvbuf[0],  //***
			recvbuflen,
			0);
		fwrite(recvbuf, sizeof(char), iResult, recv_file);  //***
		cumul += iResult; //***
	}


	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return;
}
