#include "processQueue.h"

void initQueue(ProcessQueue* q){
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void push(ProcessQueue* q, PCB* process){
    if (!q || !process) return;

    if (isQueueEmpty(q)) {
        // First item in the queue
        q->head = process;
        q->tail = process;
        process->prev = NULL;
    } else {
        // Add to the end
        process->prev = q->tail;
        q->tail->next = process;
        q->tail = process;
    }

    // Always true for the new tail
    process->next = NULL;
    q->size++;
}

PCB* pop(ProcessQueue* q){
    if (isQueueEmpty(q)) return NULL;

    PCB* headProcess = q->head;
    if(!headProcess->next) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = headProcess->next;
        q->head->prev = NULL;
    }

    // Safety
    headProcess->next = NULL; 
    headProcess->prev = NULL; 

    q->size--;

    return headProcess;
}

int isQueueEmpty(ProcessQueue* q){
    return q->size == 0;
}

void removeProcess(ProcessQueue* q, PCB* process){
    if (!q || !process || isQueueEmpty(q)) return;

    if (process->prev != NULL)
        process->prev->next = process->next;
    else
        q->head = process->next;

    if (process->next != NULL)
        process->next->prev = process->prev;
    else
        q->tail = process->prev;

    process->next = NULL;
    process->prev = NULL;
    q->size--;
}

