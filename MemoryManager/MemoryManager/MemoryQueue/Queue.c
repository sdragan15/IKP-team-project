#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

typedef struct queueElement {
	void *next;
	void *value;
} element;

typedef struct queueHeader {
	element *front;
	element *rear;
} header;

static header* create() {
	header* handle = (header*)malloc(sizeof(*handle));
	handle->front = NULL;
	handle->rear = NULL;

	return handle;
}

static void push(header* h, void* elem) {
	element* el = (element*) malloc(sizeof(*el));
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
}

static void* pop(header* h) {
	element* popEl = h->front;

	if (popEl == NULL) { 
		return NULL;
	}
	else {
		h->front = (element *)popEl->next;
		void* value = popEl->value;
		free(popEl);

		return value;
	}
}
