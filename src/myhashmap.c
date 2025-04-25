#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myhashmap.h"

// djb2 hash function
static unsigned long hashString(const char *str) {
    unsigned long hash = 5381;
    while (*str)
        hash = ((hash << 5) + hash) + *str++;
    return hash;
}

void initHashMap(HashMap *map, int size) {
    map->size = size;
    map->entries = calloc(size, sizeof(HashEntry));
}

void setHashMap(HashMap *map, const char *key, long file_size, long compressed_size) {
    unsigned long hash = hashString(key);
    int index = hash % map->size;

    // printf("Setting value for key: %s\n", key);
    // printf("Hash value: %lu, Index: %d\n", hash, index);

    for (int i = 0; i < map->size; i++) {
        int try = (index + i) % map->size;

        if (!map->entries[try].used || strcmp(map->entries[try].key, key) == 0) {
            if (!map->entries[try].used) {
                map->entries[try].key = strdup(key);
                // printf("Added new entry for key: %s at index %d\n", key, try);
            }

            map->entries[try].file_size = file_size;
            map->entries[try].compressed_size = compressed_size;
            map->entries[try].used = 1;

            // printf("Updated entry for key: %s with file_size: %ld and compressed_size: %ld\n", key, file_size, compressed_size);
            return;
        }
    }
    fprintf(stderr, "HashMap full or collision resolution failed for key: %s\n", key);
}

int getHashMap(HashMap *map, const char *key, long *file_size, long *compressed_size) {
    unsigned long hash = hashString(key);
    int index = hash % map->size;

    for (int i = 0; i < map->size; i++) {
        int try = (index + i) % map->size;

        if (map->entries[try].used) {
            if (strcmp(map->entries[try].key, key) == 0) {
                *file_size = map->entries[try].file_size;
                *compressed_size = map->entries[try].compressed_size;
                return 1;
            }
        } else {
            printf("No entry found for %s at index %d\n", key, try);
            return 0;
        }
    }

    printf("Key not found: %s\n", key);
    return 0;
}


void freeHashMap(HashMap *map) {
    for (int i = 0; i < map->size; i++) {
        if (map->entries[i].used)
            free(map->entries[i].key);
    }
    free(map->entries);
}
