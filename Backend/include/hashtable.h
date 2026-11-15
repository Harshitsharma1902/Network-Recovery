#ifndef HASHTABLE_H
#define HASHTABLE_H

#define HASH_SIZE 101

typedef struct HashNode {

    int key;
    int value;

    struct HashNode *next;

} HashNode;

typedef struct {
    HashNode *buckets[HASH_SIZE];
} HashTable;

void initHash(HashTable *h);
void insertHash(HashTable *h, int key, int value);
int searchHash(HashTable *h, int key);
void deleteHash(HashTable *h, int key);
void freeHash(HashTable *h);

#endif