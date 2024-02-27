#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"

/*
Client:
    Initialize Winsock
    Create a socket
    Connect to the server
    Send and receive data
    Disconnect
*/

void main() {
    // Initialisation
    WSADATA wsaData;
    int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);  // winsock ver 2.2
    if (startupResult != 0) {
        printf("ERROR: WSAStartup failed %ld", WSAGetLastError());
    }

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    int getaddrResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (getaddrResult != 0) {
        printf("ERROR: getaddrinfo failed: %d\n%s\n", getaddrResult, strerror(getaddrResult));
        WSACleanup();
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    // Attempt to connect to the first address returned by the call to getaddrinfo
    ptr = result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
    ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
    }

    // Connect to server.
    int connectResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (connectResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }
    freeaddrinfo(result);
    
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
    }

    char *msgToServer = "HELLO?!??!";
    int sendResult = send(ConnectSocket, msgToServer, (int) strlen(msgToServer), 0);
    if (sendResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
    }
    printf("Sent '%s' to server", msgToServer);

    // Recieve back message sent
    char recieved[1024];
    int recvResult = recv(ConnectSocket, recieved, 1024, 0);
    printf("\n\nRecieved %d bytes from server:\n%s\n\n", recvResult, recieved);

    // shutdown connection
    int shutdownResult = shutdown(ConnectSocket, SD_SEND);
    if (shutdownResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
}