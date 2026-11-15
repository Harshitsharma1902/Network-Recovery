#ifndef ROUTER_H
#define ROUTER_H

typedef struct {
    int id;
    char name[50];
    char ip[20];
    int active;
} Router;

#endif