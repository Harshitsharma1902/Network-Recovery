#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_SIZE 100

typedef struct {
    int type;
    int u;
    int v;
} NetworkEvent;

typedef struct {
    NetworkEvent data[QUEUE_SIZE];
    int front;
    int rear;
    int count;
} Queue;

void initQueue(Queue *q);
int enqueue(Queue *q, NetworkEvent event);
int dequeue(Queue *q, NetworkEvent *event);

#endif