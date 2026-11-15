#include <stdlib.h>
#include "hashtable.h"

static int hashFunction(int key) {
    return key % HASH_SIZE;
}

void initHash(HashTable *h) {

    for(int i = 0; i < HASH_SIZE; i++)
        h->buckets[i] = NULL;
}

void insertHash(HashTable *h, int key, int value) {

    int index = hashFunction(key);

    HashNode *temp = h->buckets[index];

    while(temp != NULL) {

        if(temp->key == key) {
            temp->value = value;
            return;
        }

        temp = temp->next;
    }

    HashNode *node = malloc(sizeof(HashNode));

    if(node == NULL)
        return;

    node->key = key;
    node->value = value;

    node->next = h->buckets[index];

    h->buckets[index] = node;
}

int searchHash(HashTable *h, int key) {

    int index = hashFunction(key);

    HashNode *temp = h->buckets[index];

    while(temp != NULL) {

        if(temp->key == key)
            return temp->value;

        temp = temp->next;
    }

    return -1;
}

void deleteHash(HashTable *h, int key) {

    int index = hashFunction(key);

    HashNode *temp = h->buckets[index];
    HashNode *previous = NULL;

    while(temp != NULL) {

        if(temp->key == key) {

            if(previous == NULL)
                h->buckets[index] = temp->next;

            else
                previous->next = temp->next;

            free(temp);
            return;
        }

        previous = temp;
        temp = temp->next;
    }
}

void freeHash(HashTable *h) {

    for(int i = 0; i < HASH_SIZE; i++) {

        HashNode *temp = h->buckets[i];

        while(temp != NULL) {

            HashNode *next = temp->next;

            free(temp);

            temp = next;
        }

        h->buckets[i] = NULL;
    }
}