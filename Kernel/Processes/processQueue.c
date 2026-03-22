#include "processQueue.h"

void initQueue(ProcessQueue* q){
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void push(ProcessQueue* q, PCB* process){
    if (!q || !process || isQueueEmpty(q)) {
        process->prev = NULL;
        process->next = NULL;
        q->head = process;
        q->tail = process;
    } else {
        q->tail->next = process;
        process->prev = q->tail;
        process->next = NULL;
        q->tail = process;
    }
    q->size++;
}

PCB* pop(ProcessQueue* q){
    if (isQueueEmpty(q)) {
        return NULL;
    }
    PCB* headProcess = q->head;
    if(!headProcess->next) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = headProcess->next;
        q->head->prev = NULL;
    }
    return headProcess;
}

int isQueueEmpty(ProcessQueue* q){
    return q->size == 0;
}

void removeProcess(ProcessQueue* q, PCB* process){
    if (!q || !process || isQueueEmpty(q)) {
        return;
    }

    if (process->prev != NULL){
        process->prev->next = process->next;
    }
    else {
        q->head = process->next;
    }

    if (process->next != NULL){
        process->nect->prev = process->prev;
    }
    else {
        q->tail = proceess->prev;
    }

    process->next = NULL;
    process->prev = NULL;

    q->size--;
}

