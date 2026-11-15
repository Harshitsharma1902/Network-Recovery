#include <stdio.h>

#include "graph.h"
#include "storage.h"
#include "server.h"

int main() {

    Graph network;

    initGraph(&network);

    int loaded = loadNetwork(&network);

    if(!loaded) {

        printf("Creating default network...\n");

        for(int i = 0; i < 6; i++) {
            addRouter(&network);
        }

        addEdge(&network, 0, 1, 5);
        addEdge(&network, 0, 2, 2);
        addEdge(&network, 1, 3, 3);
        addEdge(&network, 2, 3, 1);
        addEdge(&network, 3, 4, 4);
        addEdge(&network, 4, 5, 2);

        saveNetwork(&network);
    }

    printf("\nNetwork loaded successfully.\n");
    printf("Total routers: %d\n", network.count);

    startServer(&network);

    return 0;
}