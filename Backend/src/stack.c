#include "stack.h"

void initStack(Stack *s) {
    s->top = -1;
}

int push(Stack *s, Failure f) {

    if(s->top == STACK_SIZE - 1)
        return 0;

    s->data[++s->top] = f;

    return 1;
}

int pop(Stack *s, Failure *f) {

    if(s->top == -1)
        return 0;

    *f = s->data[s->top--];

    return 1;
}