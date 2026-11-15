#include "queue.h"

void initQueue(Queue *q) {

    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

int enqueue(Queue *q, NetworkEvent event) {

    if(q->count == QUEUE_SIZE)
        return 0;

    q->rear = (q->rear + 1) % QUEUE_SIZE;

    q->data[q->rear] = event;

    q->count++;

    return 1;
}

int dequeue(Queue *q, NetworkEvent *event) {

    if(q->count == 0)
        return 0;

    *event = q->data[q->front];

    q->front = (q->front + 1) % QUEUE_SIZE;

    q->count--;

    return 1;
}