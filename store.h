#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 256

#define TABLE_SIZE 100

// 구조체 정의
typedef struct Entry {
    char *key;
    char *value;
} Entry;

Entry *table[TABLE_SIZE];

// 해시 함수
unsigned int hash(char *key);
void process_command(char *command);
void put(char *key, char *value);
void get(char *key);
