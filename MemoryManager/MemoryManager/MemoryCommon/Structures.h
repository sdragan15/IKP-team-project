#pragma once

typedef struct clientRequest {
	int command;
	int numOfBytes; // alocate memory
	unsigned short portOfClient;
	void* memoryFree; //free memory
} request;

typedef struct serverResponse {
	int statusCode;
	int numOfBytes;
	unsigned short portOfClient;
	char* memoryStart;
} response;
