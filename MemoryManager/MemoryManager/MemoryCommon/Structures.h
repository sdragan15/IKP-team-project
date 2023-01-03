#pragma once

typedef struct clientRequest {
	int command;
	int numOfBytes; // alocate memory
	unsigned short portOfClient;
	//char* memoryStart; //free memory
} request;

typedef struct serverResponse {
	int statusCode;
	char* portOfClient;
	char* memoryStart;
} response;
