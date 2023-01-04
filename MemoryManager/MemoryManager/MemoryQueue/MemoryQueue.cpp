// UDP server that use blocking sockets
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "Structures.h"
#include "Queue.cpp"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define QUEUE_PORT 15000	// Port number of server that will be used for communication with clients
#define BUFFER_SIZE 512		// Size of buffer that will be used for sending and receiving messages to clients

#define SERVER_IP_ADDRESS "127.0.0.1"		// IPv4 address of server
#define SERVER_PORT 15002

int main()
{
    sockaddr_in queueAddress;
    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    memset((char*)&queueAddress, 0, sizeof(queueAddress));
    queueAddress.sin_family = AF_INET; 			// set server address protocol family
    queueAddress.sin_addr.s_addr = INADDR_ANY;		// use all available addresses of server
    queueAddress.sin_port = htons(QUEUE_PORT);

    SOCKET queueSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_DGRAM,   // datagram socket
        IPPROTO_UDP); // UDP

    if (queueSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    int iResult = bind(queueSocket, (SOCKADDR*)&queueAddress, sizeof(queueAddress));

    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(queueSocket);
        WSACleanup();
        return 1;
    }

    printf("Simple UDP server started and waiting client messages.\n");

    while (1)
    {
        sockaddr_in clientAddress;
        sockaddr_in serverAddress;

        memset(&clientAddress, 0, sizeof(clientAddress));
        memset(&serverAddress, 0, sizeof(serverAddress));
        memset(dataBuffer, 0, BUFFER_SIZE);

        serverAddress.sin_family = AF_INET;								// IPv4 address famly
        serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// Set server IP address using string
        serverAddress.sin_port = htons(SERVER_PORT);

        int sockAddrLen = sizeof(clientAddress);

        header* handle = create();
        
        iResult = recvfrom(queueSocket,				// Own socket
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

        char ipAddress[16]; // 15 spaces for decimal notation (for example: "192.168.100.200") + '\0'

        strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(clientAddress.sin_addr));

        unsigned short clientPort = ntohs(clientAddress.sin_port);

        printf("Dodavanje u kju\n");
        request* podac = (request*)dataBuffer;
        podac->portOfClient = clientPort;
        push(handle, podac);

        printf("Vadjenje iz kjua\n");
        void* izKjua = pop(handle); //vadimo podatke iz kjua
        request* podaci = (request*)izKjua;

        request client;
        client.numOfBytes = 1;
        client.command = htonl(1);

        iResult = sendto(queueSocket,						// Own socket
            (char*)podaci,						// Text of message
            sizeof(request),				// Message size
            0,									// No flags
            (SOCKADDR*)&serverAddress,		// Address structure of server (type, IP address and port)
            sizeof(serverAddress));

        if (iResult == SOCKET_ERROR)
        {
            printf("sendto failed with error: %d\n", WSAGetLastError());
            closesocket(queueSocket);
            WSACleanup();
            return 1;
        }


        printf("%d %d %d\n", ntohl(podaci->command), ntohl(podaci->numOfBytes), podaci->portOfClient);
        printf("Client connected from ip: %s, port: %d, sent: %s.\n", ipAddress, clientPort, dataBuffer);

    }

    iResult = closesocket(queueSocket);
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