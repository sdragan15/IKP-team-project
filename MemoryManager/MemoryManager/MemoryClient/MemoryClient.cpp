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
#define SERVER_PORT 15000					// Port number of server that will be used for communication with clients
#define BUFFER_SIZE 512						// Size of buffer that will be used for sending and receiving messages to client

int userMenu() {
    char dataBuffer[BUFFER_SIZE];
    int command;
    int numOfBytes;

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

        printf("\t\tEnter number of bytes to allocate:");
        gets_s(dataBuffer, BUFFER_SIZE);
        numOfBytes = htonl(atoi(dataBuffer));

        if (!(command >= 1 && command <= 3)) {
            printf("\t\tChoice is not valid, try again!\n");
        }
    } while (!(command >= 1 && command <= 3));

    return numOfBytes;
}

/*request* allocate_memory() {
    request reqClient;
    int temp = -1;
    char buf[BUFFER_SIZE];
    //popunjavamo strukturu koju cemo slati ka redu
    printf("\t\tEnter number of bytes to allocate:");
    gets_s(buf, BUFFER_SIZE);

    reqClient.command = htonl(1);
    temp = atoi(buf);
    reqClient.numOfBytes = htonl(temp);


    //reqClient.portOfClient = (char*)malloc(strlen("123") + 1);
    //memcpy(reqClient.portOfClient, "1234" + '\0', 4);

    return &reqClient;
}*/

void free_memory() {
    //popunjavamo strukturu koju cemo slati ka redu
}

/*char* userChoice() {
    int command = userMenu();
    request* req = allocate_memory();

    request *reqq = *req;
    /*switch (command) {
    case 1:
        req = allocate_memory();
        break;
    case 2:
        req = allocate_memory();
        break;
    }*/

    //return (char)reqq;
//}*/


int main()
{
    // Server address structure
    sockaddr_in serverAddress;

    // Size of server address structure
    int sockAddrLen = sizeof(serverAddress);

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
    memset((char*)&serverAddress, 0, sizeof(serverAddress));

    // Initialize address structure of server
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
    struct sockaddr_in sin = {};
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
    printf("%u\n", port);

    //kraj iscitavanja porta
    while (1) {
        //memcpy(dataBuffer, userChoice(), sizeof(dataBuffer));
        request client;
        client.numOfBytes = userMenu();
        client.command = htonl(1);
        //strcpy_s(client.portOfClient, 4, "123");

        // Read string from user into outgoing buffer
        //gets_s(dataBuffer, BUFFER_SIZE);

        // Send message to server
        ////char* data = userMenu();
        //request *r = (request*)data;
        //request* d = (request*) data;
       // printf("%d\n", ntohl(d->numOfBytes));
        //printf("%d\n", ntohl(d->command));
        iResult = sendto(clientSocket,						// Own socket
            (char*)&client,						// Text of message
            sizeof(request),				// Message size
            0,									// No flags
            (SOCKADDR*)&serverAddress,		// Address structure of server (type, IP address and port)
            sizeof(serverAddress));			// Size of sockadr_in structure

        // Check if message is succesfully sent. If not, close client application
        if (iResult == SOCKET_ERROR)
        {
            printf("sendto failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
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
