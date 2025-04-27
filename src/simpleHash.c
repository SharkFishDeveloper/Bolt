#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simpleHash.h"

// djb2 Hash Function
unsigned long hash_function(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash;
}

// Create a new hashmap
HashMap *create_hashmap() {
    HashMap *map = malloc(sizeof(HashMap));
    if (map) {
        for (int i = 0; i < HASHMAP_SIZE; i++) {
            map->buckets[i] = NULL;
        }
    }
    return map;
}

// Insert key-value pair
void hashmap_insert_key(HashMap *map, const char *key, const char *value) {
    unsigned long hash = hash_function(key) % HASHMAP_SIZE;
    Entry *entry = map->buckets[hash];

    // Check if key exists and update
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            free(entry->value);
            entry->value = strdup(value);
            return;
        }
        entry = entry->next;
    }

    // Key not found, insert new entry at beginning
    Entry *new_entry = malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    new_entry->next = map->buckets[hash];
    map->buckets[hash] = new_entry;
}

// Get value by key
char *hashmap_get_key(HashMap *map, const char *key) {
    unsigned long hash = hash_function(key) % HASHMAP_SIZE;
    Entry *entry = map->buckets[hash];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;  // Not found
}

// Delete a key
void hashmap_delete_key(HashMap *map, const char *key) {
    unsigned long hash = hash_function(key) % HASHMAP_SIZE;
    Entry *entry = map->buckets[hash];
    Entry *prev = NULL;

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[hash] = entry->next;
            } else {
                prev->next = entry->next;
            }
            free(entry->key);
            free(entry->value);
            free(entry);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

// Free the entire hashmap
void free_hashmap(HashMap *map) {
    for (int i = 0; i < HASHMAP_SIZE; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(map);
}