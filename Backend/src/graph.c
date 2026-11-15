#include <stdio.h>
#include "graph.h"

void initGraph(Graph *g) {

    g->count = 0;

    for(int i = 0; i < MAX_ROUTERS; i++) {

        for(int j = 0; j < MAX_ROUTERS; j++) {

            g->weight[i][j] = INF;
            g->failed[i][j] = 0;
        }

        g->weight[i][i] = 0;
    }
}

int addRouter(Graph *g) {

    if(g->count >= MAX_ROUTERS)
        return -1;

    int id = g->count++;

    for(int i = 0; i < MAX_ROUTERS; i++) {
        g->weight[id][i] = INF;
        g->weight[i][id] = INF;
        g->failed[id][i] = 0;
        g->failed[i][id] = 0;
    }

    g->weight[id][id] = 0;

    return id;
}

void addEdge(Graph *g, int u, int v, int weight) {

    if(u < 0 || v < 0 ||
       u >= g->count || v >= g->count ||
       u == v || weight <= 0)
        return;

    g->weight[u][v] = weight;
    g->weight[v][u] = weight;

    g->failed[u][v] = 0;
    g->failed[v][u] = 0;
}

void removeEdge(Graph *g, int u, int v) {

    if(u < 0 || v < 0 ||
       u >= g->count || v >= g->count ||
       u == v)
        return;

    g->weight[u][v] = INF;
    g->weight[v][u] = INF;

    g->failed[u][v] = 0;
    g->failed[v][u] = 0;
}

void failEdge(Graph *g, int u, int v) {

    if(u < 0 || v < 0 ||
       u >= g->count || v >= g->count)
        return;

    if(g->weight[u][v] != INF) {

        g->failed[u][v] = 1;
        g->failed[v][u] = 1;
    }
}

void recoverEdge(Graph *g, int u, int v) {

    if(u < 0 || v < 0 ||
       u >= g->count || v >= g->count)
        return;

    g->failed[u][v] = 0;
    g->failed[v][u] = 0;
}

void failRouter(Graph *g, int router) {

    if(router < 0 || router >= g->count)
        return;

    for(int i = 0; i < g->count; i++) {

        g->failed[router][i] = 1;
        g->failed[i][router] = 1;
    }
}

void recoverRouter(Graph *g, int router) {

    if(router < 0 || router >= g->count)
        return;

    for(int i = 0; i < g->count; i++) {

        g->failed[router][i] = 0;
        g->failed[i][router] = 0;
    }
}

void dijkstra(Graph *g, int src, int dest) {

    int dist[MAX_ROUTERS];
    int visited[MAX_ROUTERS] = {0};
    int previous[MAX_ROUTERS];

    for(int i = 0; i < g->count; i++) {

        dist[i] = INF;
        previous[i] = -1;
    }

    dist[src] = 0;

    for(int i = 0; i < g->count; i++) {

        int u = -1;
        int minimum = INF;

        for(int j = 0; j < g->count; j++) {

            if(!visited[j] && dist[j] < minimum) {

                minimum = dist[j];
                u = j;
            }
        }

        if(u == -1)
            break;

        visited[u] = 1;

        for(int v = 0; v < g->count; v++) {

            if(g->weight[u][v] != INF &&
               !g->failed[u][v] &&
               !g->failed[u][u] &&
               !g->failed[v][v] &&
               !visited[v]) {

                int newDist = dist[u] + g->weight[u][v];

                if(newDist < dist[v]) {

                    dist[v] = newDist;
                    previous[v] = u;
                }
            }
        }
    }

    if(dist[dest] == INF) {

        printf("NO_ROUTE\n");
        return;
    }

    int path[MAX_ROUTERS];
    int size = 0;

    for(int current = dest;
        current != -1;
        current = previous[current]) {

        path[size++] = current;
    }

    printf("ROUTE: ");

    for(int i = size - 1; i >= 0; i--) {

        printf("R%d", path[i]);

        if(i != 0)
            printf(" -> ");
    }

    printf("\nCOST: %d\n", dist[dest]);
}

int bfs(Graph *g, int src, int dest) {

    if(src < 0 || dest < 0 ||
       src >= g->count || dest >= g->count)
        return 0;

    if(g->failed[src][src] || g->failed[dest][dest])
        return 0;

    int visited[MAX_ROUTERS] = {0};
    int queue[MAX_ROUTERS];

    int front = 0;
    int rear = 0;

    queue[rear++] = src;
    visited[src] = 1;

    while(front < rear) {

        int u = queue[front++];

        if(u == dest)
            return 1;

        for(int v = 0; v < g->count; v++) {

            if(g->weight[u][v] != INF &&
               !g->failed[u][v] &&
               !g->failed[v][v] &&
               !visited[v]) {

                visited[v] = 1;
                queue[rear++] = v;
            }
        }
    }

    return 0;
}