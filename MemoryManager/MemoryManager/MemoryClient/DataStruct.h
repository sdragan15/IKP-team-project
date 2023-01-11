#ifndef DATASTRUCT_H
#define DATASTRUCT_H


#define CAPACITY 50000 // Size of the Hash Table

// Define the Hash Table Item here
typedef struct Ht_item {
    int key;
    unsigned int value;
} Ht_item;


// Define the Hash Table here
typedef struct HashTable {
    // Contains an array of pointers
    // to items
    Ht_item** items;
    int size;
    int count;
} HashTable;

unsigned int hash_function(int key);
Ht_item* create_item(char* key, char* value);
HashTable* create_table(int size);
void free_item(Ht_item* item);
void free_table(HashTable* table);
void handle_collision(HashTable* table, unsigned long index, Ht_item* item);
void ht_insert(HashTable* table, int key, char* value);
char* ht_search(HashTable* table, int key);
void print_search(HashTable* table, int key);
void print_table(HashTable* table);
void ht_delete(HashTable* table, int key);

#endif
