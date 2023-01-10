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
#include "manager.h"
#include "queue.c"


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 15002	// Port number of server that will be used for communication with clients
#define BUFFER_SIZE 512		// Size of buffer that will be used for sending and receiving messages to clients
#define CLIENT_IP_ADDRESS "127.0.0.1"
#define CLIENT_SLEEP_TIME 50

char* memory;
header* queueFree = NULL;
header* queueProcess = NULL;

HANDLE semaphoreFree;
HANDLE semaphoreProcess;

char* allocate_mem(int bytes) {
    char* response = alocate_memory(memory, bytes);
    print_memory(memory);
    return response;
}

void free_mem(char* start) {
    free_memory(memory, start);
    print_memory(memory);
}

void handle_request(request* data) {
    switch (ntohl(data->command)) {
    case 1:
        if (queueProcess != NULL) {
            push(queueProcess, data, semaphoreProcess);
        }
        break;
    case 2:
        if (queueFree != NULL) {
            push(queueFree, data, semaphoreFree);
        }
        break;
    }

    return 0;
}

void free_response() {
    struct sockaddr_in serverAddress;
    int sockAddrLen = sizeof(serverAddress);
    char dataBuffer[BUFFER_SIZE];
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    memset((char*)&serverAddress, 0, sizeof(serverAddress));

    SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_DGRAM,   // Datagram socket
        IPPROTO_UDP); // UDP protocol

    if (clientSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in sin;
    socklen_t slen;
    int sock;
    short unsigned int port;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = 0;

    iResult = bind(clientSocket, (struct sockaddr*)&sin, sizeof(sin));

    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    unsigned long int nonBlockingMode = 1;
    iResult = ioctlsocket(clientSocket, FIONBIO, &nonBlockingMode);
    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    FD_SET set;
    struct timeval timeVal;

    FD_ZERO(&set);
    FD_SET(clientSocket, &set);

    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    while (1) {
        request* data = pop(queueFree, semaphoreFree);


        if (data != NULL) {
            free_mem(data->memoryFree);

            int client_port = ntohs(data->portOfClient);

            serverAddress.sin_family = AF_INET;								// IPv4 address famly
            serverAddress.sin_addr.s_addr = inet_addr(CLIENT_IP_ADDRESS);	// Set server IP address using string
            serverAddress.sin_port = htons(client_port);					// Set server port

            response* responseData = (response*)malloc(sizeof(response));

            responseData->statusCode = ntohl(1);

            while (1) {
                iResult = select(0 /* ignored */, &set, &set, NULL, &timeVal);

                if (iResult == SOCKET_ERROR)
                {
                    fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
                    continue;
                }

                if (iResult == 0)
                {
                    Sleep(CLIENT_SLEEP_TIME);
                    continue;
                }

                iResult = sendto(clientSocket,						// Own socket
                    (char*)responseData,						// Text of message
                    sizeof(response),				// Message size
                    0,									// No flags
                    (SOCKADDR*)&serverAddress,		// Address structure of server (type, IP address and port)
                    sizeof(serverAddress));			// Size of sockadr_in structure


                if (iResult == SOCKET_ERROR)
                {
                    printf("sendto failed with error: %d\n", WSAGetLastError());
                    closesocket(clientSocket);
                    WSACleanup();
                    return 1;
                }

                break;
            }
        }
        Sleep(1000);
    }
}

void process_response() {
    struct sockaddr_in serverAddress;
    int sockAddrLen = sizeof(serverAddress);
    char dataBuffer[BUFFER_SIZE];
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    memset((char*)&serverAddress, 0, sizeof(serverAddress));

    SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_DGRAM,   // Datagram socket
        IPPROTO_UDP); // UDP protocol

    if (clientSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in sin;
    socklen_t slen;
    int sock;
    short unsigned int port;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = 0;

    iResult = bind(clientSocket, (struct sockaddr*)&sin, sizeof(sin));

    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    unsigned long int nonBlockingMode = 1;
    iResult = ioctlsocket(clientSocket, FIONBIO, &nonBlockingMode);
    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    FD_SET set;
    struct timeval timeVal;

    FD_ZERO(&set);
    FD_SET(clientSocket, &set);

    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    while (1) {
        request* data = pop(queueProcess, semaphoreProcess);
        
        if (data != NULL) {
            int bytes = ntohl(data->numOfBytes);
            int client_port = ntohs(data->portOfClient);
            char* res = allocate_mem(bytes);
           

            serverAddress.sin_family = AF_INET;								// IPv4 address famly
            serverAddress.sin_addr.s_addr = inet_addr(CLIENT_IP_ADDRESS);	// Set server IP address using string
            serverAddress.sin_port = htons(client_port);					// Set server port

            while (1) {
                iResult = select(0 /* ignored */, &set, &set, NULL, &timeVal);

                if (iResult == SOCKET_ERROR)
                {
                    fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
                    continue;
                }

                if (iResult == 0)
                {
                    Sleep(CLIENT_SLEEP_TIME);
                    continue;
                }

                response* responseData = (response*)malloc(sizeof(response));
                int a = 20;
                responseData->memoryStart = res;
                

                iResult = sendto(clientSocket,						// Own socket
                    (char*)responseData,						// Text of message
                    sizeof(response),				// Message size
                    0,									// No flags
                    (SOCKADDR*)&serverAddress,		// Address structure of server (type, IP address and port)
                    sizeof(serverAddress));			// Size of sockadr_in structure

                
                if (iResult == SOCKET_ERROR)
                {
                    printf("sendto failed with error: %d\n", WSAGetLastError());
                    closesocket(clientSocket);
                    WSACleanup();
                    return 1;
                }

                break;
            }
        }
        Sleep(1000);
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

    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    memset(dataBuffer, 0, BUFFER_SIZE);

    int sockAddrLen = sizeof(clientAddress);

    HANDLE sendThread;
    DWORD sendThreadId;

    HANDLE processThread;
    DWORD processThreadId;

    sendThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)free_response,
        NULL,
        0,
        &sendThreadId);

    processThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)process_response,
        NULL,
        0,
        &processThreadId
    );

    queueFree = create(semaphoreFree);
    queueProcess = create(semaphoreProcess);

    while (1)
    {
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

        //printf("Received message from: %s, port: %d, sent: %s.\n", ipAddress, clientPort, dataBuffer);
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

