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

char* get(char *key) {
    unsigned int index = hash(key);
    Entry *entry = table[index];

    if (entry) {
        // printf("%s\n", entry->value);
    }

    return entry->value;
}

void append_log(char *command) {
    FILE *fp = fopen("log.txt", "a");
    LogEntry new_entry;

    int n = fseek(fp, -sizeof(LogEntry), SEEK_END);
    new_entry.index = 0;

    printf("fseek: %d\n", n);
    if (n != -1) {
        fread(&new_entry, sizeof(LogEntry), 1, fp);

        new_entry.index += 1;
    }

    new_entry.command = strdup(command);

    printf("Appending log entry: %d %s\n", new_entry.index, new_entry.command);
    fwrite(&new_entry, sizeof(LogEntry), 1, fp);

    // 디스크에 쓰기 저장 시점 고민
    fflush(fp);
    fclose(fp);
}
