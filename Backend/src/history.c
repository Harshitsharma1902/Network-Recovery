#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "history.h"

void addHistory(EventNode **head, int id, const char *description) {

    EventNode *newNode = malloc(sizeof(EventNode));

    if(newNode == NULL)
        return;

    newNode->id = id;

    snprintf(newNode->description,
             sizeof(newNode->description),
             "%s", description);

    newNode->next = NULL;

    if(*head == NULL) {

        *head = newNode;
        return;
    }

    EventNode *temp = *head;

    while(temp->next != NULL)
        temp = temp->next;

    temp->next = newNode;
}

void displayHistory(EventNode *head) {

    EventNode *temp = head;

    while(temp != NULL) {

        printf("%d: %s\n",
               temp->id,
               temp->description);

        temp = temp->next;
    }
}

void freeHistory(EventNode *head) {

    while(head != NULL) {

        EventNode *temp = head;

        head = head->next;

        free(temp);
    }
}