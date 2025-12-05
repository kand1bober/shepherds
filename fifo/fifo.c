#include "fifo.h"
#include <stdlib.h>
#include <stdio.h>

// Инициализация
void fifo_init(FIFO *q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
}

// Проверка на пустоту
bool fifo_empty(FIFO *q) {
    return q->count == 0;
}

// Проверка на переполнение (для связного списка нет ограничения)
bool fifo_full(FIFO *q) {
    return false; // всегда можно добавить, пока память позволяет
}

// Добавление элемента, возвращает указатель на новый элемент
Node* fifo_enqueue(FIFO *q, int value) {
    Node *node = malloc(sizeof(Node));
    if (!node) return NULL; // ошибка выделения памяти
    node->value = value;
    node->next = NULL;

    if (q->tail) {
        q->tail->next = node;
        q->tail = node;
    } else {
        q->head = q->tail = node;
    }

    q->count++;
    return node; // возвращаем указатель на добавленный элемент
}

// Извлечение элемента
bool fifo_dequeue(FIFO *q, int *value) {
    if (fifo_empty(q)) return false;
    Node *node = q->head;
    *value = node->value;
    q->head = node->next;
    if (!q->head) q->tail = NULL;
    free(node);
    q->count--;
    return true;
}

// Очистка очереди
void fifo_free(FIFO *q) {
    Node *cur = q->head;
    while (cur) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    q->head = q->tail = NULL;
    q->count = 0;
}

// int main() {
//     FIFO q;
//     fifo_init(&q);

//     // Добавляем элементы и получаем указатели на них
//     Node *node1 = fifo_enqueue(&q, 10);
//     Node *node2 = fifo_enqueue(&q, 20);
//     Node *node3 = fifo_enqueue(&q, 30);

//     // Теперь можно работать с указателями на элементы
//     // Например, можно изменять значения напрямую:
//     // node2->value = 25;

//     printf("%d\n", node1 == q.head );

//     printf("Содержимое очереди:\n");
//     int val;
//     while(fifo_dequeue(&q, &val)) {
//         printf("%d\n", val);
//     }

//     fifo_free(&q);
//     return 0;
// }
