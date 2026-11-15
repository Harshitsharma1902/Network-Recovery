#ifndef HISTORY_H
#define HISTORY_H

typedef struct EventNode {

    int id;
    char description[100];

    struct EventNode *next;

} EventNode;

void addHistory(EventNode **head, int id, const char *description);
void displayHistory(EventNode *head);
void freeHistory(EventNode *head);

#endif