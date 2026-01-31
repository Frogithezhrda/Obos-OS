#ifndef HEAP_H
#define HEAP_H

#include "paging.h"

#define FREE 1
#define OCCUPIED 0
#define NULL 0

typedef struct HeapBlock
 {
    unsigned long size;
    unsigned int free;
    struct HeapBlock* next;
} HeapBlock;



typedef struct Heap 
{
    void* start;
    void* end;
    unsigned long size;
    HeapBlock* firstBlock;
} Heap;

#define HEAP_BLOCK_HEADER_SIZE sizeof(HeapBlock)
#define ALIGN_SIZE 16

void* kmalloc(unsigned long size);
void kfree(void* ptr);

void* umalloc(unsigned long size);
void ufree(void* ptr);

void initKernelHeap(void);
void initUserHeap(void);

#endif