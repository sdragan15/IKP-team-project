// UDP server that use blocking sockets
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#include "conio.h"
#include "Structures.h"
#include "memory.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 15002	// Port number of server that will be used for communication with clients
#define BUFFER_SIZE 512		// Size of buffer that will be used for sending and receiving messages to clients

char* memory;

void allocate_mem(int bytes) {
    alocate_memory(memory, bytes);
    print_memory(memory);
}

void free_mem(char* start) {

}

void handle_request(request* data) {
    switch (ntohl(data->command)) {
    case 1:
        allocate_mem(ntohl(data->numOfBytes));
        break;
    case 2:
        break;
    }
}

int main()
{
    memory = (char*)malloc(100);
    init_memory(memory, 0);

    struct sockaddr_in serverAddress;
    char dataBuffer[BUFFER_SIZE];
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; 			// set server address protocol family
    serverAddress.sin_addr.s_addr = INADDR_ANY;		// use all available addresses of server
    serverAddress.sin_port = htons(SERVER_PORT);

    SOCKET serverSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_DGRAM,   // datagram socket
        IPPROTO_UDP); // UDP

    if (serverSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    int iResult = bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));

    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Simple UDP server started and waiting client messages.\n");

    while (1)
    {
        struct sockaddr_in clientAddress;
        memset(&clientAddress, 0, sizeof(clientAddress));
        memset(dataBuffer, 0, BUFFER_SIZE);

        int sockAddrLen = sizeof(clientAddress);
       
        iResult = recvfrom(serverSocket,				// Own socket
            dataBuffer,					// Buffer that will be used for receiving message
            BUFFER_SIZE,					// Maximal size of buffer
            0,							// No flags
            (SOCKADDR*)&clientAddress,	// Client information from received message (ip address and port)
            &sockAddrLen);				// Size of sockadd_in structure

        if (iResult == SOCKET_ERROR)
        {
            printf("recvfrom failed with error: %d\n", WSAGetLastError());
            continue;
        }

        dataBuffer[iResult] = '\0';

        char ipAddress[16];

        strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(clientAddress.sin_addr));

        unsigned short clientPort = ntohs(clientAddress.sin_port);

        request* podac = (request*)dataBuffer;
        handle_request(podac);

        printf("%d %d %d\n", ntohl(podac->command), ntohl(podac->numOfBytes), podac->portOfClient);
        printf("Client connected from ip: %s, port: %d, sent: %s.\n", ipAddress, clientPort, dataBuffer);
    }

    iResult = closesocket(serverSocket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Server successfully shut down.\n");

    WSACleanup();

    return 0;
}

