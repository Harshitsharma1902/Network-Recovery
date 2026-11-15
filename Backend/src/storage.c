#include <stdio.h>

#include "storage.h"

#define CONNECTION_FILE "data/connections.txt"
#define EVENT_FILE "data/events.txt"


// SAVE NETWORK
int saveNetwork(Graph *g) {

    FILE *fp = fopen(CONNECTION_FILE, "w");

    if(fp == NULL) {
        printf("Unable to save network.\n");
        return 0;
    }

    fprintf(fp, "%d\n", g->count);

    for(int i = 0; i < g->count; i++) {

        for(int j = i + 1; j < g->count; j++) {

            if(g->weight[i][j] != INF) {

                fprintf(fp, "%d %d %d %d\n",
                    i,
                    j,
                    g->weight[i][j],
                    g->failed[i][j]);
            }
        }
    }

    fclose(fp);

    return 1;
}


// LOAD NETWORK
int loadNetwork(Graph *g) {

    FILE *fp = fopen(CONNECTION_FILE, "r");

    if(fp == NULL)
        return 0;

    int count;

    if(fscanf(fp, "%d", &count) != 1 ||
       count < 0 || count > MAX_ROUTERS) {

        fclose(fp);
        return 0;
    }

    g->count = count;

    for(int i = 0; i < MAX_ROUTERS; i++) {

        for(int j = 0; j < MAX_ROUTERS; j++) {

            g->weight[i][j] = (i == j) ? 0 : INF;
            g->failed[i][j] = 0;
        }
    }

    int u, v, weight, failed;

    while(fscanf(fp, "%d %d %d %d",
                 &u, &v, &weight, &failed) == 4) {

        if(u < 0 || v < 0 ||
           u >= count || v >= count ||
           u == v || weight <= 0)
            continue;

        g->weight[u][v] = weight;
        g->weight[v][u] = weight;

        g->failed[u][v] = failed != 0;
        g->failed[v][u] = failed != 0;
    }

    fclose(fp);

    return 1;
}


// SAVE EVENT HISTORY
int saveHistory(EventNode *head) {

    FILE *fp = fopen(EVENT_FILE, "w");

    if(fp == NULL)
        return 0;

    EventNode *temp = head;

    while(temp != NULL) {

        fprintf(fp, "%d\t%s\n",
            temp->id,
            temp->description);

        temp = temp->next;
    }

    fclose(fp);

    return 1;
}


// LOAD EVENT HISTORY
int loadHistory(EventNode **head, int *nextId) {

    FILE *fp = fopen(EVENT_FILE, "r");

    if(fp == NULL)
        return 0;

    char line[150];

    int id;
    char description[100];

    while(fgets(line, sizeof(line), fp) != NULL) {

        if(sscanf(line, "%d\t%99[^\n]",
                  &id, description) == 2) {

            addHistory(head, id, description);

            if(id >= *nextId)
                *nextId = id + 1;
        }
    }

    fclose(fp);

    return 1;
}