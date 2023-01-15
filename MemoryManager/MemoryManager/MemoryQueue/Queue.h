#ifndef QUEUE_H
#define QUEUE_H

HANDLE semaphore;

typedef struct queueElement {
	void* next;
	void* value;
} element;

typedef struct queueHeader {
	element* front;
	element* rear;
} header;

header* create(HANDLE semaphore);
void push(header* h, void* elem, HANDLE semaphore);
void* pop(header* h, HANDLE semaphore);

#endif

