#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "responseHandler.c"

#define DEFAULT_PORT "8080"
#define REQUEST_SIZE 1024
#define SHOW_LOGS 1

/*
Server:
    Initialize Winsock
    Create a socket
    Bind the socket
    Listen on the socket for a client
    Accept a connection from a client
    Receive and send data
    Disconnect
*/

void closeAndCleanup(int ServerSocket) {
    closesocket(ServerSocket);
    WSACleanup();
}

void printLine() {
    printf("\n");
    for (int i = 0; i < 80; i++) {
        printf("=");
    }
    printf("\n");
}

void printInBlock(char *msg) {
    printLine();
    printf("%s", msg);
    printLine();
}

int clientConnectionHandler(int ServerSocket) {
    // Accept a client socket
    SOCKET ClientSocket = INVALID_SOCKET;
    ClientSocket = accept(ServerSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("ERROR: accept failed: %d\n", WSAGetLastError());
        closeAndCleanup(ServerSocket);
        return 0;
    }

    // Recieve message from client
    char request[REQUEST_SIZE];
    memset(request, 0, REQUEST_SIZE);
    int bytesRecieved = recv(ClientSocket, request, REQUEST_SIZE, 0);
    printf("\nConnection found, recieved (%d) bytes from client", bytesRecieved);
    if (SHOW_LOGS) {
        printInBlock(request);
    }

    if (bytesRecieved > 0) {
        // Send back to the client
        struct HTTPResponse response = getResponse(request);
        int sentHeader = send(ClientSocket, response.header.contents, response.header.size, 0);
        int sentBody = send(ClientSocket, response.body.contents, response.body.size, 0);

        if (sentHeader == SOCKET_ERROR || sentBody == SOCKET_ERROR) {
            printf("send failed: %d (header: %d, body %d)\n", WSAGetLastError(), sentHeader, sentBody);
            closeAndCleanup(ServerSocket);
        }
        printf("\nSent back (%d) bytes to the client", sentBody + sentHeader);
        if (SHOW_LOGS) {
            printInBlock(response.body.contents);
        }

        // Cleanup response
        free(response.body.contents);
        free(response.header.contents);
    }

    // Shutdown connection
    int shutdownResult = shutdown(ClientSocket, SD_SEND);
    if (shutdownResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closeAndCleanup(ServerSocket);
    }

    // Cleanup connection
    closesocket(ClientSocket);
    if (SHOW_LOGS) {
        printf("Closed connection with client\n\n");
    }
    return 1;
}

int createAndBindSocket(struct addrinfo *result) {
    // Create a socket
    SOCKET ServerSocket = INVALID_SOCKET;
    ServerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ServerSocket == INVALID_SOCKET) {
        printf("ERROR: socket creation failed %ld\n", WSAGetLastError());
        closeAndCleanup(ServerSocket);
        freeaddrinfo(result);
    }

    // Bind the socket
    int bindResult = bind( ServerSocket, result->ai_addr, (int)result->ai_addrlen);
    if (bindResult == SOCKET_ERROR) {
        printf("ERROR: socket binf failed %ld\n", WSAGetLastError());
        closeAndCleanup(ServerSocket);
    }
    return ServerSocket;
}

void listenToSocket(int ServerSocket) {
    if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("ERROR: listen failed with error: %ld\n", WSAGetLastError());
        closeAndCleanup(ServerSocket);
    }
}

void main() {
    int dbInit = initialiseDB();
    // Initialisation
    WSADATA wsaData;
    int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);  // winsock ver 2.2
    if (startupResult != 0) {
        printf("ERROR: WSAStartup failed %ld", WSAGetLastError());
    }

    // Resolve the local address and port to be used by the server
    struct addrinfo *result = NULL, hints;
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0) {
        printf("Error: failed to get addr info %ld", WSAGetLastError());
        WSACleanup();
    }

    // Create server socket (then free unneeded result)
    int ServerSocket = createAndBindSocket(result);
    freeaddrinfo(result);

    // Listen to socket for incoming connections
    listenToSocket(ServerSocket);
    printf("Server setup complete!\nListening for clients...\n\n");
    
    // Accept connection loop
    while (1) {
        if (clientConnectionHandler(ServerSocket)) {
            continue;
        }
        break;
    }

    // End server
    closeAndCleanup(ServerSocket);
    printf("Closed server\n");
}
