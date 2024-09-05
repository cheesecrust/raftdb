#include "store.h"

// 해시 함수
unsigned int hash(char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash << 5) + *key++;
    }
    return hash % TABLE_SIZE;
}

void put(char *key, char *value) {
    unsigned int index = hash(key);
    Entry *new_entry = malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    table[index] = new_entry;
}

void get(char *key) {
    unsigned int index = hash(key);
    Entry *entry = table[index];

    if (entry) {
        printf("%s\n", entry->value);
    }
}

