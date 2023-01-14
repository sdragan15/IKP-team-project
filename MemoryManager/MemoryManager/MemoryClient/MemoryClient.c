// UDP client that uses blocking sockets
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Structures.h"
#include "DataStruct.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"		// IPv4 address of server
#define QUEUE_PORT 15000					// Port number of server that will be used for communication with clients
#define SERVER_PORT 15002
#define BUFFER_SIZE 512						// Size of buffer that will be used for sending and receiving messages to client
#define SERVER_SLEEP_TIME 50

int counter = 0;
int usersChoice = -1;
char* arrayOfMemory[10];

int allocate_memory();
int print_all_allocated_memory(HashTable* ht);

request* userMenu(HashTable* ht) {
    char dataBuffer[BUFFER_SIZE];
    int command;
    int numOfBytes;
    request* reqClient = (request*)malloc(sizeof(request));

    do {
        printf("------------- User menu -------------\n");
        printf("\tChoose an option: \n");
        printf("\t\t1. Alocate memory\n");
        printf("\t\t2. Free memory\n");
        printf("\t\t3. Show statistics\n");
        printf("\t\t4. Exit program\n");
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
            reqClient->command = htonl(2);
            reqClient->memoryFree = htonl(print_all_allocated_memory(ht));
            //printf("aaaa %d\n", *arrayOfMemory[0]);
            //reqClient->numOfBytes = htonl(4);

        }
        else if (command == 3) {
            reqClient->command = htonl(3);
        }
        else if (command == 4) {
            reqClient->command = -1;
        }
        else {
            printf("\t\tChoice is not valid, try again!\n");
        }    
    } while (!(command >= 1 && command <= 4));
 
    return reqClient;
}


int allocate_memory() {
    char dataBuffer[BUFFER_SIZE];
    printf("\t\tEnter number of bytes to allocate:");
    gets_s(dataBuffer, BUFFER_SIZE);
    return atoi(dataBuffer);
}

int print_all_allocated_memory(HashTable* ht) {
    //popunjavamo strukturu koju cemo slati ka redu
    char dataBuffer[BUFFER_SIZE];
    /*for (int i = 0; i < 10; i++) {
        printf("[%d]: %lu\n", i, arrayOfMemory[i]);
    }*/
    print_table(ht);
    printf("\t\tChoose memory from hash table:");
    gets_s(dataBuffer, BUFFER_SIZE);

    //return arrayOfMemory[atoi(dataBuffer)];
    usersChoice = atoi(dataBuffer); //pamti se kljuc memorije koju je klijent trazio da moze da se oslobodi iz tabele
    return (int)ht_search(ht, atoi(dataBuffer));
}

int main()
{
    HashTable* ht = create_table(CAPACITY);
    struct sockaddr_in queueAddress;
    struct sockaddr_in serverAddress;

    int sockAddrLen = sizeof(queueAddress);

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    memset((char*)&queueAddress, 0, sizeof(queueAddress));
    memset((char*)&serverAddress, 0, sizeof(serverAddress));

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

    iResult = bind(clientSocket, (struct sockaddr*)&sin, sizeof(sin));

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

    printf("Client port: %d\n\n", port);

    while (1) {
        request* client = userMenu(ht);
        if (client->command == -1) {
            printf("User doesn't want more request. Shut down...\n");
            break;
        }
        client->portOfClient = htons(port);

        iResult = sendto(clientSocket,						// Own socket
            (char*)client,						// Text of message
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


        char dataBufferRecv[BUFFER_SIZE];
        memset(dataBufferRecv, 0, BUFFER_SIZE);

        printf("Waiting for receive...\n");

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

        if (podac->statusCode == ntohl(1)) {
            printf("Server successfully freed memory\n");
            ht_delete(ht, usersChoice);
        }
        else if (podac->statusCode == ntohl(2)) {
            printf("Server showed statistics.\n");
        }
        else {
            printf("Server successfully allocated memory. Address of memory: %d.\n", ntohl(podac->memoryStart));
            ht_insert(ht, counter++, ntohl(podac->memoryStart));
        }
        //arrayOfMemory[counter++] = ntohl(podac->memoryStart);    
    }
    
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

    WSACleanup();

    return 0;
}
