#ifndef GRAPH_H
#define GRAPH_H

#define MAX_ROUTERS 50
#define INF 999999

typedef struct {
    int weight[MAX_ROUTERS][MAX_ROUTERS];
    int failed[MAX_ROUTERS][MAX_ROUTERS];
    int count;
} Graph;

void initGraph(Graph *g);

int addRouter(Graph *g);
void addEdge(Graph *g, int u, int v, int weight);
void removeEdge(Graph *g, int u, int v);

void failEdge(Graph *g, int u, int v);
void recoverEdge(Graph *g, int u, int v);

void failRouter(Graph *g, int router);
void recoverRouter(Graph *g, int router);

void dijkstra(Graph *g, int src, int dest);
int bfs(Graph *g, int src, int dest);

#endif
