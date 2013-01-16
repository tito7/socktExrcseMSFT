// Taken from http://msdn.microsoft.com/en-us/library/windows/desktop/ms738545(v=vs.85).aspx

//====================== S T E P S  T O  C R E A T E  A  T C P  S O C K E T  S E R V E R ===============
// To communicate using the transport layer protocol TCP, use the operating system provided API socket interface
// The socket API is programmed at the application layer uses combinations of the API functions in accordance to  
// the sequence as defined by the API to communicate using the TCP transport layer protocol.
// Thus concepts such as sockets which can be either a client or server.
// The client and server are different roles and therfore the operations used differ although there is some 
// degree of commonality

// ======================== C L I E N T  S O C K E T  M O D E L  ===========================================

// ================ C L I E N T  A N D  S E R V E R  S O C K E T  C O M M O N  O P E R A T I O N S =========
// ===========  1 ) Initialise winsoc
// ===========  2 ) Create a socket (of client type)
// ================ a ) Declare addrinfo
// ================ b ) Call addrinfo
// ================ c ) Call socket() with returned info from addressinfo

// ================================= C L I E N T  S P E C I F I C  S O C K E T  O P E R A T I O N S =======
// ===========  3 ) Connect to a socket
// ===========  4 ) Send and receive data
// ===========  5 ) Disconnect client

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

//This program will set up a TCP client using the winsock2 API
//As a client, it can connect to another socket (endpoint) specified by
//an IP address and a port number.
//The port number is dfined by a macro constant and the IP address specified by the
//first argument to the program
int __cdecl main(int argc, char **argv) 
{
	    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    WSADATA wsaData;
    int iResult;
	//================================== Initialise Winsock2 WSDATA ============================
    // Initialize Winsock - this step is needed for both the client and server
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }


    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *resAddrInfoPtr = NULL,
                    *ptr = NULL,
                    hints;
    char *sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];

    int recvbuflen = DEFAULT_BUFLEN;
    


    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;		//IPv4 or IPv6 is possible
    hints.ai_socktype = SOCK_STREAM;	//For UDP, adifferent value is specified
    hints.ai_protocol = IPPROTO_TCP;

	//=================================== Get the address info ==================================
    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &resAddrInfoPtr);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }


    // Attempt to connect to an address until one succeeds
    for(ptr=resAddrInfoPtr; ptr != NULL ;ptr=ptr->ai_next) {
	//=================================== Create a socket and try to connect ===================================
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(resAddrInfoPtr);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

	//======================================= Send data to the socket (address/ port) connected to =====================
    // Send an initial buffer
    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    //=============================================== Receive until the peer closes the connection =================
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
