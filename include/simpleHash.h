
#ifndef SIMPLEHASH_H
#define SIMPLEHASH_H


// Define the hashmap size
#define HASHMAP_SIZE 1009

// Entry struct
typedef struct Entry {
    char *key;
    char *value;
    struct Entry *next;
} Entry;

// HashMap struct
typedef struct HashMap {
    Entry *buckets[HASHMAP_SIZE];
} HashMap;

// Function declarations
HashMap *create_hashmap();
void hashmap_insert_key(HashMap *map, const char *key, const char *value);
char *hashmap_get_key(HashMap *map, const char *key);
void hashmap_delete_key(HashMap *map, const char *key);
void free_hashmap(HashMap *map);


#endif
            