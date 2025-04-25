#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct {
    char *key;               // file path
    long file_size;
    long compressed_size;
    int used;
} HashEntry;

typedef struct {
    HashEntry *entries;
    int size;
} HashMap;

void initHashMap(HashMap *map, int size);
void setHashMap(HashMap *map, const char *key, long file_size, long compressed_size);
int getHashMap(HashMap *map, const char *key, long *file_size, long *compressed_size);
void freeHashMap(HashMap *map);

#endif // HASHMAP_H
