#include <stdlib.h>  
#include <stdio.h>
#include "memory.h"

int MEMORY_SIZE = 100;
const int BLOCK_SIZE = 5;
const int HEADER_SIZE = 1;
const char FREE_CHR = '_';
const char OCCUPIED_CHR = '0';
const char HEADER_CHR = 'X';
const int INCREASE_MEMORY = 20;

void init_memory(char* memory, int start) {
	int i = 0;
	for (i = start; i < MEMORY_SIZE; i++) {
		memory[i] = FREE_CHR;
	}
}

void print_memory(char* memory) {
	printf("\n----------------Memory------------------\n\n");
	int i = 0;
	int columns = 5;
	for (i = 0; i < MEMORY_SIZE; i++) {
		if (i % BLOCK_SIZE == 0 && i != 0) {
			printf(" ");
		}
		if (i != 0 && i % (columns * BLOCK_SIZE) == 0) {
			printf("\n");
		}
		if (memory[i] == '\0') {
			printf("%c", OCCUPIED_CHR);
		}
		else {
			printf("%c", memory[i]);
		}

	}
	printf("\n\n");
}

void update_memory(char* memory, int from, int to, char value) {
	int i = from;
	int count = 0;
	for (i = from; i <= to; i++) {
		if (count < HEADER_SIZE) {
			memory[i] = HEADER_CHR;
			count++;
		}
		else {
			memory[i] = value;
		}

	}
}

char* alocate_memory(char* memory, int bytes) {
	if (bytes <= 0) {
		printf("Number of bytes must be greater than 0");
		return NULL;
	}

	int total_memory = bytes + HEADER_SIZE;

	int block_start = 0;

	int i = 0;
	int count = 0;
	int res = 0;
	for (i = 0; i < MEMORY_SIZE; i++) {
		if (memory[i] == FREE_CHR) {
			if (count == 0) {
				if (i % BLOCK_SIZE == 0) {
					block_start = 1;
				}
				else {
					block_start = 0;
				}
			}
			count++;

			if (i != 0 && i % BLOCK_SIZE == 0 && block_start == 0) {
				count = 1;
				block_start = 1;
			}

			if (count >= total_memory) {
				res = i - total_memory + 1;
				update_memory(memory, res, i, OCCUPIED_CHR);
				print_memory(memory);
				return memory + res + HEADER_SIZE;
			}
		}
		else {
			count = 0;
		}

	}

	MEMORY_SIZE += INCREASE_MEMORY;
	memory = (char*)realloc(memory, MEMORY_SIZE);
	init_memory(memory, MEMORY_SIZE - INCREASE_MEMORY);
	printf("Expanding memory...");
	return alocate_memory(memory, bytes);
}

void free_memory(char* memory, char* start) {
	char* i;
	start -= HEADER_SIZE;
	for (i = start; i < start + HEADER_SIZE; i++) {
		*i = FREE_CHR;
	}

	for (i = start + HEADER_SIZE; i < memory + MEMORY_SIZE; i++) {
		if (*i != FREE_CHR && *i != HEADER_CHR) {
			*i = FREE_CHR;
		}
		else {
			print_memory(memory);
			return;
		}
	}
}

void print_statistics(char* memory) {
	int i;
	int free = 0;
	int occupied = 0;
	int largest_free_block = 0;
	int temp_free = 0;
	for (i = 0; i < MEMORY_SIZE; i++) {
		if (memory[i] == FREE_CHR) {
			free++;
			temp_free++;
		}
		else {
			occupied++;
			if (largest_free_block < temp_free) {
				largest_free_block = temp_free;
			}
			temp_free = 0;
		}
	}
	if (largest_free_block < temp_free) {
		largest_free_block = temp_free;
	}
	temp_free = 0;

	float fregmentation = (free - largest_free_block) / (1.0 * free) * 100;

	printf("---------------------------Statistics-------------------------");
	printf("\nNumber of free bytes: %d\n", free);
	printf("Number of occupied bytes: %d\n", occupied);
	printf("Memory fragmentation: %.2lf%%\n", fregmentation);
	printf("--------------------------------------------------------------");
}
