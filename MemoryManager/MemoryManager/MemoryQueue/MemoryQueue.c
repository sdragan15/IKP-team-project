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
#include "Queue.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define QUEUE_PORT 15000	// Port number of queue that will be used for communication with clients
#define BUFFER_SIZE 512		// Size of buffer that will be used for sending and receiving messages to clients

#define SERVER_IP_ADDRESS "127.0.0.1"		// IPv4 address of server
#define SERVER_PORT 15002

#define THREAD_SLEEP 500

header* handle = NULL;

HANDLE semaphore;

void send_item() {
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
        void* izKjua = pop(handle, semaphore); //vadimo podatke iz kjua
        if (izKjua == NULL) {
            Sleep(THREAD_SLEEP);
            printf(".");
            continue;
        }
        request* podaci = (request*)izKjua;

        request client;
        client.numOfBytes = 1;
        client.command = htonl(1);

        printf("Sending from %d\n", ntohs(podaci->portOfClient));

        serverAddress.sin_family = AF_INET;								// IPv4 address famly
        serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// Set server IP address using string
        serverAddress.sin_port = htons(SERVER_PORT);					// Set server port

        while (1) {
            iResult = select(0 /* ignored */, &set, &set, NULL, &timeVal);

            if (iResult == SOCKET_ERROR)
            {
                fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
                printf("Waiting for sent...\n");
                continue;
            }

            if (iResult == 0)
            {
                Sleep(THREAD_SLEEP);
                continue;
            }

            iResult = sendto(clientSocket,						// Own socket
                (char*)podaci,						// Text of message
                sizeof(response),				// Message size
                0,									// No flags
                (SOCKADDR*)&serverAddress,		// Address structure of server (type, IP address and port)
                sizeof(serverAddress));			// Size of sockadr_in structure

            printf("Successfully sent.\n");

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
}

void receive_item() {
    struct sockaddr_in queueAddress;
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

    int sockAddrLen = sizeof(queueAddress);

    while (1)
    {
        printf("Waiting for message...\n");

        iResult = recvfrom(queueSocket,				// Own socket
            dataBuffer,					// Buffer that will be used for receiving message
            BUFFER_SIZE,					// Maximal size of buffer
            0,							// No flags
            (SOCKADDR*)&queueAddress,	// Client information from received message (ip address and port)
            &sockAddrLen);				// Size of sockadd_in structure

        if (iResult == SOCKET_ERROR)
        {
            printf("recvfrom failed with error: %d\n", WSAGetLastError());
            continue;
        }

        printf("Received message.\n");

        dataBuffer[iResult] = '\0';

        char ipAddress[16]; // 15 spaces for decimal notation (for example: "192.168.100.200") + '\0'

        strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(queueAddress.sin_addr));

        unsigned short clientPort = ntohs(queueAddress.sin_port);

        printf("Dodavanje u kju\n");
        request* podac = (request*)dataBuffer;

        request* message = (request*)malloc(sizeof(request));
        message->command = podac->command;
        message->memoryFree = podac->memoryFree;
        message->numOfBytes = podac->numOfBytes;
        message->portOfClient = podac->portOfClient;

        push(handle, podac, semaphore);
    }

    iResult = closesocket(queueSocket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Queue receive socket successfully shut down.\n");

    WSACleanup();

}

int main()
{
    HANDLE sendThread;
    DWORD sendThreadId;

    HANDLE receiveThread;
    DWORD receiveThreadId;

    sendThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)send_item,
        NULL,
        0,
        &sendThreadId);

    receiveThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)receive_item,
        NULL,
        0,
        &receiveThreadId
    );

    handle = create(semaphore);
    HANDLE threads[2] = { receiveThread, sendThread };

    WaitForMultipleObjects(2, threads, FALSE, INFINITE);

    CloseHandle(semaphore);
    CloseHandle(receiveThread);
    CloseHandle(sendThread);

    return 0;
}