#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "Queue.h"


header* create(HANDLE semaphore) {
	semaphore = CreateSemaphore(0, 1, 1, NULL);

	header* handle = (header*)malloc(sizeof(*handle));
	handle->front = NULL;
	handle->rear = NULL;

	return handle;

}

void push(header* h, void* elem, HANDLE semaphore) {
	WaitForSingleObject(semaphore, INFINITE);
	element* el = (element*)malloc(sizeof(*el));
	el->next = NULL;
	el->value = elem;

	//is list empty
	if (h->front == NULL) {
		h->front = el;
		h->rear = el;
	}
	else { //list is not empty, rewire
		element* oldTail = h->rear;
		oldTail->next = el;
		h->rear = el;
	}
	ReleaseSemaphore(semaphore, 1, NULL);

}

void* pop(header* h, HANDLE semaphore) {
	WaitForSingleObject(semaphore, INFINITE);
	element* popEl = h->front;

	if (popEl == NULL) {
		ReleaseSemaphore(semaphore, 1, NULL);
		return NULL;
	}
	else {
		h->front = (element*)popEl->next;
		void* value = popEl->value;
		free(popEl);
		ReleaseSemaphore(semaphore, 1, NULL);
		return value;
	}

}
