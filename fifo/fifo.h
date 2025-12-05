#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int count;
} FIFO;

void fifo_init(FIFO *q);
bool fifo_empty(FIFO *q);
bool fifo_full(FIFO *q);
Node* fifo_enqueue(FIFO *q, int value); // Возвращает указатель на новый элемент
bool fifo_dequeue(FIFO *q, int *value);
void fifo_free(FIFO *q);

#endif
