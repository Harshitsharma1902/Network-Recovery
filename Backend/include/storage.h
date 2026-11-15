#ifndef STORAGE_H
#define STORAGE_H

#include "graph.h"
#include "history.h"

int saveNetwork(Graph *g);
int loadNetwork(Graph *g);

int saveHistory(EventNode *head);
int loadHistory(EventNode **head, int *nextId);

#endif