#ifndef MEMORY_H
#define MEMORY_H

void init_memory(char* memory, int start);
void print_memory(char* memory);
void update_memory(char* memory, int from, int to, char value);
char* alocate_memory(char* memory, int bytes);
void free_memory(char* memory, char* start);
void print_statistics(char* memory);


#endif // MEMORY_H
