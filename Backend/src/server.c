#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mongoose.h"
#include "server.h"
#include "storage.h"
#include "stack.h"
#include "history.h"

static Graph *network;

static Stack undoStack;

static EventNode *history = NULL;

static int eventCounter = 1;


// SEND JSON RESPONSE
static void sendJSON(struct mg_connection *c,
                     const char *json) {

    mg_http_reply(c, 200,
        "Content-Type: application/json\r\n",
        "%s", json);
}


// HANDLE HTTP REQUESTS
static void handler(struct mg_connection *c,
                    int ev,
                    void *ev_data) {

    if(ev != MG_EV_HTTP_MSG)
        return;

    struct mg_http_message *hm = ev_data;


    // --------------------------------
    // GET NETWORK STATE
    // --------------------------------

    if(mg_match(hm->uri, mg_str("/api/state"), NULL)) {

        char response[65536];

        int pos = snprintf(response, sizeof(response),
            "{\"count\":%d,\"edges\":[",
            network->count);

        int first = 1;

        for(int i = 0; i < network->count; i++) {

            for(int j = i + 1; j < network->count; j++) {

                if(network->weight[i][j] != INF) {

                    int written = snprintf(
                        response + pos,
                        sizeof(response) - pos,

                        "%s{\"u\":%d,\"v\":%d,\"weight\":%d,\"failed\":%s}",

                        first ? "" : ",",
                        i,
                        j,
                        network->weight[i][j],
                        network->failed[i][j] ? "true" : "false"
                    );

                    if(written < 0 ||
                       (size_t) written >= sizeof(response) - pos) {

                        sendJSON(c, "{\"error\":\"Response too large\"}");
                        return;
                    }

                    pos += written;

                    first = 0;
                }
            }
        }

        snprintf(response + pos,
            sizeof(response) - pos, "]}");

        sendJSON(c, response);
    }


    // --------------------------------
    // FAIL LINK
    // --------------------------------

    else if(mg_match(hm->uri,
                     mg_str("/api/link/fail"), NULL)) {

        long u = mg_json_get_long(hm->body, "$.u", -1);
        long v = mg_json_get_long(hm->body, "$.v", -1);

        if(u < 0 || v < 0 ||
           u >= network->count ||
           v >= network->count ||
           u == v ||
           network->weight[u][v] == INF) {

            sendJSON(c, "{\"error\":\"Invalid link\"}");
            return;
        }

        if(!network->failed[u][v]) {

            Failure f = {1, (int)u, (int)v};

            if(!push(&undoStack, f)) {

                sendJSON(c, "{\"error\":\"Undo stack full\"}");
                return;
            }

            failEdge(network, (int)u, (int)v);

            char description[100];

            snprintf(description, sizeof(description),
                "Link R%ld-R%ld failed", u, v);

            addHistory(&history, eventCounter++, description);

            saveNetwork(network);
            saveHistory(history);
        }

        sendJSON(c, "{\"success\":true}");
    }


    // --------------------------------
    // RECOVER LINK
    // --------------------------------

    else if(mg_match(hm->uri,
                     mg_str("/api/link/recover"), NULL)) {

        long u = mg_json_get_long(hm->body, "$.u", -1);
        long v = mg_json_get_long(hm->body, "$.v", -1);

        if(u < 0 || v < 0 ||
           u >= network->count ||
           v >= network->count ||
           u == v ||
           network->weight[u][v] == INF) {

            sendJSON(c, "{\"error\":\"Invalid link\"}");
            return;
        }

        if(network->failed[u][v]) {

            recoverEdge(network, (int)u, (int)v);

            char description[100];

            snprintf(description, sizeof(description),
                "Link R%ld-R%ld recovered", u, v);

            addHistory(&history, eventCounter++, description);

            saveNetwork(network);
            saveHistory(history);
        }

        sendJSON(c, "{\"success\":true}");
    }


    // --------------------------------
    // FIND SHORTEST ROUTE
    // --------------------------------

    else if(mg_match(hm->uri,
                     mg_str("/api/route"), NULL)) {

        long src = mg_json_get_long(hm->body, "$.src", -1);
        long dest = mg_json_get_long(hm->body, "$.dest", -1);

        if(src < 0 || dest < 0 ||
           src >= network->count ||
           dest >= network->count) {

            sendJSON(c, "{\"error\":\"Invalid router\"}");
            return;
        }

        if(!bfs(network, (int)src, (int)dest)) {

            sendJSON(c, "{\"reachable\":false}");
            return;
        }

        int dist[MAX_ROUTERS];
        int visited[MAX_ROUTERS] = {0};
        int previous[MAX_ROUTERS];

        for(int i = 0; i < network->count; i++) {

            dist[i] = INF;
            previous[i] = -1;
        }

        dist[src] = 0;


        // DIJKSTRA ALGORITHM

        for(int i = 0; i < network->count; i++) {

            int u = -1;
            int minimum = INF;

            for(int j = 0; j < network->count; j++) {

                if(!visited[j] && dist[j] < minimum) {

                    minimum = dist[j];
                    u = j;
                }
            }

            if(u == -1)
                break;

            visited[u] = 1;

            for(int v = 0; v < network->count; v++) {

                if(network->weight[u][v] != INF &&
                   !network->failed[u][v] &&
                   !network->failed[u][u] &&
                   !network->failed[v][v] &&
                   !visited[v] &&
                   dist[u] + network->weight[u][v] < dist[v]) {

                    dist[v] = dist[u] + network->weight[u][v];

                    previous[v] = u;
                }
            }
        }


        // BUILD ROUTE

        int path[MAX_ROUTERS];

        int size = 0;

        for(int current = (int)dest;
            current != -1;
            current = previous[current]) {

            path[size++] = current;
        }


        // CREATE JSON RESPONSE

        char response[2048];

        int pos = snprintf(response, sizeof(response),

            "{\"reachable\":true,\"cost\":%d,\"route\":[",

            dist[dest]
        );

        for(int i = size - 1; i >= 0; i--) {

            int written = snprintf(
                response + pos,
                sizeof(response) - pos,

                "%s%d",

                i == size - 1 ? "" : ",",
                path[i]
            );

            if(written < 0 ||
               (size_t) written >= sizeof(response) - pos) {

                sendJSON(c, "{\"error\":\"Response too large\"}");
                return;
            }

            pos += written;
        }

        snprintf(response + pos,
            sizeof(response) - pos, "]}");

        sendJSON(c, response);
    }


    // --------------------------------
    // UNDO LAST FAILURE
    // --------------------------------

    else if(mg_match(hm->uri,
                     mg_str("/api/undo"), NULL)) {

        Failure f;

        if(!pop(&undoStack, &f)) {

            sendJSON(c, "{\"error\":\"Nothing to undo\"}");
            return;
        }

        if(f.type == 1) {

            recoverEdge(network, f.u, f.v);
        }

        else if(f.type == 2) {

            recoverRouter(network, f.u);
        }

        saveNetwork(network);

        addHistory(&history, eventCounter++,
                   "Last failure undone");

        saveHistory(history);

        sendJSON(c, "{\"success\":true}");
    }


    // --------------------------------
    // ADD ROUTER
    // --------------------------------

    else if(mg_match(hm->uri, mg_str("/api/router/add"), NULL)) {

        int id = addRouter(network);

        if(id < 0) {
            sendJSON(c, "{\"error\":\"Max routers limit reached\"}");
            return;
        }

        char description[100];
        snprintf(description, sizeof(description), "Router R%d provisioned", id);

        addHistory(&history, eventCounter++, description);

        saveNetwork(network);
        saveHistory(history);

        char response[128];
        snprintf(response, sizeof(response), "{\"success\":true,\"id\":%d,\"count\":%d}", id, network->count);
        sendJSON(c, response);
    }


    // --------------------------------
    // ADD / EDIT EDGE (LINK)
    // --------------------------------

    else if(mg_match(hm->uri, mg_str("/api/edge/save"), NULL) ||
            mg_match(hm->uri, mg_str("/api/link/save"), NULL)) {

        long u = mg_json_get_long(hm->body, "$.u", -1);
        long v = mg_json_get_long(hm->body, "$.v", -1);
        long weight = mg_json_get_long(hm->body, "$.weight", -1);

        if(u < 0 || v < 0 || weight <= 0 ||
           u >= network->count || v >= network->count || u == v) {

            sendJSON(c, "{\"error\":\"Invalid edge parameters\"}");
            return;
        }

        int isEdit = (network->weight[u][v] != INF);

        addEdge(network, (int)u, (int)v, (int)weight);

        char description[100];
        snprintf(description, sizeof(description),
                 "Link R%ld-R%ld %s (weight: %ld)",
                 u, v, isEdit ? "updated" : "provisioned", weight);

        addHistory(&history, eventCounter++, description);

        saveNetwork(network);
        saveHistory(history);

        char response[128];
        snprintf(response, sizeof(response),
                 "{\"success\":true,\"u\":%ld,\"v\":%ld,\"weight\":%ld}",
                 u, v, weight);
        sendJSON(c, response);
    }


    // --------------------------------
    // DELETE EDGE (LINK)
    // --------------------------------

    else if(mg_match(hm->uri, mg_str("/api/edge/delete"), NULL) ||
            mg_match(hm->uri, mg_str("/api/link/delete"), NULL)) {

        long u = mg_json_get_long(hm->body, "$.u", -1);
        long v = mg_json_get_long(hm->body, "$.v", -1);

        if(u < 0 || v < 0 ||
           u >= network->count || v >= network->count || u == v) {

            sendJSON(c, "{\"error\":\"Invalid link parameters\"}");
            return;
        }

        removeEdge(network, (int)u, (int)v);

        char description[100];
        snprintf(description, sizeof(description), "Link R%ld-R%ld deleted", u, v);

        addHistory(&history, eventCounter++, description);

        saveNetwork(network);
        saveHistory(history);

        sendJSON(c, "{\"success\":true}");
    }


    // --------------------------------
    // UNKNOWN ENDPOINT
    // --------------------------------

    else {

        mg_http_reply(c, 404,
            "Content-Type: application/json\r\n",
            "{\"error\":\"Not found\"}");
    }
}


// START SERVER
void startServer(Graph *g) {

    network = g;

    initStack(&undoStack);

    // Load previous event history
    loadHistory(&history, &eventCounter);

    struct mg_mgr mgr;

    mg_mgr_init(&mgr);

    if(mg_http_listen(&mgr,
                      "http://0.0.0.0:8080",
                      handler,
                      NULL) == NULL) {

        printf("Unable to start server\n");

        mg_mgr_free(&mgr);

        return;
    }

    printf("\nNetwork Recovery Backend running!\n");
    printf("Server: http://localhost:8080\n");

    for(;;) {

        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
}