#include <stdlib.h>  
#include <stdio.h>

const int MEMORY_SIZE = 100;
const int BLOCK_SIZE = 5;
const int HEADER_SIZE = 1;
const char FREE_CHR = '_';
const char OCCUPIED_CHR = '1';
const char HEADER_CHR = 'X';


void init_memory(char* memory) {
	int i = 0;
	for (i = 0; i < MEMORY_SIZE; i++) {
		memory[i] = FREE_CHR;
	}
}

void print_memory(char* memory) {
	printf("\n ----------------------------------Memory-----------------------------------------\n\n");
	int i = 0;
	for (i = 0; i < MEMORY_SIZE; i++) {
		if (i % BLOCK_SIZE == 0) {
			printf(" ");
		}
		printf("%c", memory[i]);
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

void alocate_memory(char* memory, int bytes) {
	if (bytes <= 0) {
		printf("Number of bytes must be greater than 0");
		return;
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
				return;
			}
		}
		else {
			count = 0;
		}
	}
}

void free_memory(char* memory, int start) {
	if (start < 0) {
		printf("\nPointer must be greater than 0\n");
		return;
	}

	if (memory[start - 1] == HEADER_CHR || memory[start] != HEADER_CHR) {
		printf("\nPointer is invalid!\n");
		return;
	}

	int i;
	for (i = start; i < start + HEADER_SIZE; i++) {
		memory[i] = FREE_CHR;
	}

	for (i = start + HEADER_SIZE; i < MEMORY_SIZE; i++) {
		if (memory[i] == OCCUPIED_CHR) {
			memory[i] = FREE_CHR;
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

int main() {

	char* memory = (char*)malloc(MEMORY_SIZE);

	init_memory(memory);
	print_memory(memory);

	alocate_memory(memory, 1);
	alocate_memory(memory, 10);
	alocate_memory(memory, 3);
	alocate_memory(memory, 3);
	alocate_memory(memory, 3);
	alocate_memory(memory, 1);

	print_statistics(memory);

	free_memory(memory, 0);
	free_memory(memory, 16);
	alocate_memory(memory, 12);
	alocate_memory(memory, 3);
	alocate_memory(memory, 1);

	print_statistics(memory);

	free_memory(memory, 30);
	print_statistics(memory);


}
