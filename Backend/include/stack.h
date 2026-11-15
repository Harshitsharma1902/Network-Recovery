#ifndef STACK_H
#define STACK_H

#define STACK_SIZE 100

typedef struct {
    int type;
    int u;
    int v;
} Failure;

typedef struct {
    Failure data[STACK_SIZE];
    int top;
} Stack;

void initStack(Stack *s);
int push(Stack *s, Failure f);
int pop(Stack *s, Failure *f);

#endif