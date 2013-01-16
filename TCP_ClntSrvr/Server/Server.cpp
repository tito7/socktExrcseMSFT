#undef UNICODE
// Taken from http://msdn.microsoft.com/en-us/library/windows/desktop/ms738545(v=vs.85).aspx
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

//====================== S T E P S  T O  C R E A T E  A  T C P  S O C K E T  S E R V E R ===============

// To communicate using the transport layer protocol TCP, use the operating system provided API socket interface
// The socket API is programmed at the application layer uses combinations of the API functions in accordance to  
// the sequence as defined by the API to communicate using the TCP transport layer protocol.
// Thus concepts such as sockets which can be either a client or server.
// The client and server are different roles and therfore the operations used differ although there is some 
// degree of commonality

// ======================== S E R V E R  S O C K E T  M O D E L  ===========================================

// ================ C L I E N T  A N D  S E R V E R  S O C K E T  C O M M O N  O P E R A T I O N S =========
// ===========  1 ) Initialise winsoc
// ===========  2 ) Create a socket (server type)
// ================ a ) Declare addrinfo
// ================ b ) Call addrinfo
// ================ c ) Call socket() with returned info from addressinfo

// ================================= S E R V E R  S P E C I F I C  S O C K E T  O P E R A T I O N S =======

// ===========  3 ) Bind  socket
// ===========  4 ) Listen on send  socket
// ===========  5 ) Accept a connection
// ===========  6 ) Receive and send data
// ===========  7 ) Disconnect server

int __cdecl main(void) 
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;



    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    struct addrinfo *result = NULL;
    struct addrinfo hints;    
    ZeroMemory(&hints, sizeof(hints));

	//NOTE the hints specify to the API what sort of connection is desired
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; // Indicates that the returned address will be used in a call to the bind function


    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }


    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

// ===========  3 ) Connect to a socket
    // Setup the TCP listening socket, binding it to the returned address info
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

// ===========  4 ) Listen on a socket
	// Normally a client would accept connections from multiple clients. One possible way is to have
	// a continuous loop listening for connection. If a connection request occcurs the application
	// calls an accept function and passes to another thread
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

// ===========  5 ) Accept a connection
    // Accept a client socket - happens only after a socket has successfully been heard
    ClientSocket = accept(ListenSocket, NULL, NULL);
	//After the client has accepted the server (as indicated by the value of the client socket this socket
	//is passed to a worker thread so that additonal connections can be heard
	//In unix this would be doen with a fork() which spawns a child process
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

// ===========  6 ) Receive and send data
    // Receive until the peer shuts down the connection
    do {

		//NOTE that the receive and the send take the same parameters and return the same values

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

        // Echo the buffer back to the sender
            iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

// ===========  7 ) Disconnect server
    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}
