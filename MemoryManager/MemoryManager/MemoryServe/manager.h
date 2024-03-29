#ifndef MANAGER_H
#define MANAGER_H

extern void init_memory(char* memory, int start);
extern void print_memory(char* memory);
extern void update_memory(char* memory, int from, int to, char value);
extern int alocate_memory(char* memory, int bytes);
extern void free_memory(char* memory, int start);
extern void print_statistics(char* memory);
extern char* test(char* mem, int bytes);
extern char* create_memory();
extern char* expand_memory(char* memory);

#endif // MANAGER_H
