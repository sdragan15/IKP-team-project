// UDP client that uses blocking sockets
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "Structures.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"		// IPv4 address of server
#define QUEUE_PORT 15000					// Port number of server that will be used for communication with clients
#define SERVER_PORT 15002
#define BUFFER_SIZE 512						// Size of buffer that will be used for sending and receiving messages to client
#define SERVER_SLEEP_TIME 50

int allocate_memory();
void print_all_allocated_memory(char* arrayAddr[10]);

request* userMenu() {
    char dataBuffer[BUFFER_SIZE];
    int command;
    int numOfBytes;
    request* reqClient = (request*)malloc(sizeof(request));
    
    int num = 10;
    int num2 = 13;
    char* arrayOfMemory[10] = {(char*)&num, (char*)&num2};


    do {
        printf("------------- User menu -------------\n");
        printf("\tChoose an option: \n");
        printf("\t\t1. Alocate memory\n");
        printf("\t\t2. Free memory\n");
        printf("\t\t3. Exit program\n");
        printf("\t\tYour choice is: ");
        gets_s(dataBuffer, BUFFER_SIZE);
        printf("-------------------------------------\n");

        command = atoi(dataBuffer); //ako je uspesna konverzija vrati se broj u int, ako nije vrati se 0      
        //reqClient.command = htonl(command);

        if (command == 1) {
            reqClient->command = htonl(1);
            reqClient->numOfBytes = htonl(allocate_memory());
        }
        else if (command == 2) {
            print_all_allocated_memory(arrayOfMemory);
            reqClient->command = htonl(2);
            reqClient->memoryFree = arrayOfMemory[0];
            //printf("aaaa %d\n", *arrayOfMemory[0]);
            reqClient->numOfBytes = htonl(4);

        }
        else if (command == 3) {
            reqClient->command = -1;
        }
        else {
            printf("\t\tChoice is not valid, try again!\n");
        }    
    } while (!(command >= 1 && command <= 3));
 
    return reqClient;
}


int allocate_memory() {
    char dataBuffer[BUFFER_SIZE];
    printf("\t\tEnter number of bytes to allocate:");
    gets_s(dataBuffer, BUFFER_SIZE);
    return atoi(dataBuffer);
}

void print_all_allocated_memory(char* arrayAddr[10]) {
    //popunjavamo strukturu koju cemo slati ka redu
    for (int i = 0; i < 2; i++) {
        printf("[%d]: %p\n", i, arrayAddr[i]);
    }

}

int main()
{
    // Server address structure
    struct sockaddr_in queueAddress;
    struct sockaddr_in serverAddress;

    // Size of server address structure
    int sockAddrLen = sizeof(queueAddress);

    // Buffer that will be used for sending and receiving messages to client
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is used to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets for this process
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Check if library is succesfully initialized
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Initialize memory for address structure
    memset((char*)&queueAddress, 0, sizeof(queueAddress));
    memset((char*)&serverAddress, 0, sizeof(serverAddress));

    // Initialize address structure of server
    queueAddress.sin_family = AF_INET;								// IPv4 address famly
    queueAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// Set server IP address using string
    queueAddress.sin_port = htons(QUEUE_PORT);					// Set server port

    serverAddress.sin_family = AF_INET;								// IPv4 address famly
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// Set server IP address using string
    serverAddress.sin_port = htons(SERVER_PORT);					// Set server port

    // Create a socket
    SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_DGRAM,   // Datagram socket
        IPPROTO_UDP); // UDP protocol

    // Check if socket creation succeeded
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //iscitavanje port-a klijenta
    struct sockaddr_in sin;
    socklen_t slen;
    int sock;
    short unsigned int port;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = 0;

    // Bind server address structure (type, port number and local address) to socket
    iResult = bind(clientSocket, (struct sockaddr*)&sin, sizeof(sin));

    // Check if socket is succesfully binded to server datas
    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    slen = sizeof(sin);
    getsockname(clientSocket, (struct sockaddr*)&sin, &slen);
    port = ntohs(sin.sin_port);
    //kraj iscitavanja porta
    
    //neblokirajuci rezim
    unsigned long int nonBlockingMode = 1;
    iResult = ioctlsocket(clientSocket, FIONBIO, &nonBlockingMode);
    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    while (1) {
        request* client = userMenu();
        if (client->command == -1) {
            printf("User doesn't want more request. Shut down...\n");
            break;
        }
        client->portOfClient = htons(port);

        // Initialize select parameters
        FD_SET set;
        struct timeval timeVal;

        FD_ZERO(&set);
        // Add socket we will wait to read from
        FD_SET(clientSocket, &set);

        // Set timeouts to zero since we want select to return
        // instantaneously
        timeVal.tv_sec = 0;
        timeVal.tv_usec = 0;

        iResult = select(0 /* ignored */, &set, &set, NULL, &timeVal);

        // lets check if there was an error during select
        if (iResult == SOCKET_ERROR)
        {
            fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
            continue;
        }

        // now, lets check if there are any sockets ready
        if (iResult == 0)
        {
            // there are no ready sockets, sleep for a while and check again
            Sleep(SERVER_SLEEP_TIME);
            continue;
        }
        

        iResult = sendto(clientSocket,						// Own socket
            (char*)*(&client),						// Text of message
            sizeof(request),				// Message size
            0,									// No flags
            (SOCKADDR*)&queueAddress,		// Address structure of server (type, IP address and port)
            sizeof(queueAddress));			// Size of sockadr_in structure

        // Check if message is succesfully sent. If not, close client application
        if (iResult == SOCKET_ERROR)
        {
            printf("sendto failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        iResult = select(0 /* ignored */, &set, &set, NULL, &timeVal);

        // lets check if there was an error during select
        if (iResult == SOCKET_ERROR)
        {
            fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
            continue;
        }

        // now, lets check if there are any sockets ready
        if (iResult == 0)
        {
            // there are no ready sockets, sleep for a while and check again
            Sleep(SERVER_SLEEP_TIME);
            continue;
        }

        char dataBufferRecv[BUFFER_SIZE];
        memset(dataBufferRecv, 0, BUFFER_SIZE);

        iResult = recvfrom(clientSocket,				// Own socket
            dataBufferRecv,					// Buffer that will be used for receiving message
            BUFFER_SIZE,					// Maximal size of buffer
            0,							// No flags
            (SOCKADDR*)&serverAddress,	// Client information from received message (ip address and port)
            &sockAddrLen);				// Size of sockadd_in structure

        if (iResult == SOCKET_ERROR)
        {
            printf("recvfrom failed with error: %d\n", WSAGetLastError());
            continue;
        }

        dataBufferRecv[iResult] = '\0';

        response* podac = (response*)dataBufferRecv;

        printf("Received message: %lu.\n", ntohl(podac->memoryStart));
    }
    

    // Only for demonstration purpose
    printf("Press any key to exit: ");
    _getch();

    // Close client application
    iResult = closesocket(clientSocket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Close Winsock library
    WSACleanup();

    // Client has succesfully sent a message
    return 0;
}
