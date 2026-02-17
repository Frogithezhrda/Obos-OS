#include "heap.h"

static Heap kernelHeap;
static Heap userHeap;

/*
we must align the block to be divisable by 16
some instructions require alignment which the compiler might use
modern compilers assume at least 16 byte alignment
and for heap metadata being better 16 aligned blocks are better for memory
*/
static unsigned long alignUpBlock(unsigned long size)
{
    return ((size + ALIGN_SIZE - 1) / ALIGN_SIZE) * ALIGN_SIZE;
}

static void splitBlock(HeapBlock* block, unsigned long size)
{
    if (block->size >= size + HEAP_BLOCK_HEADER_SIZE + ALIGN_SIZE)
    {
        HeapBlock* newBlock = (HeapBlock*)((char*)block + size);
        newBlock->size = block->size - size;
        newBlock->free = FREE;
        newBlock->next = block->next;
        
        block->size = size;
        block->next = newBlock;
    }
}

static HeapBlock* findFreeBlock(Heap* heap, unsigned long size)
{
    HeapBlock* current = heap->firstBlock;
    
    while (current)
    {
        if (current->free && current->size >= size) return current;
        current = current->next;
    }
    
    return NULL;
}

static void mergeBlocks(Heap* heap)
{
    HeapBlock* current = heap->firstBlock;
    
    while (current && current->next)
    {
        if (current->free && current->next->free)
        {
            current->size += current->next->size;
            current->next = current->next->next;
        }
        else
        {
            current = current->next;
        }
    }
}

void* kmalloc(unsigned long size)
{
    if (size <= 0) return NULL;
    unsigned long totalSize = alignUpBlock(size + HEAP_BLOCK_HEADER_SIZE);
    HeapBlock* block = findFreeBlock(&kernelHeap, totalSize);
    if (!block)
    {
        print("kmalloc failed: out of memory (requested ", RED);
        printNumber(size, RED);
        printLine(" bytes)", RED);
        return NULL;
    }
    splitBlock(block, totalSize);
    block->free = OCCUPIED;

    return (void*)((char*)block + HEAP_BLOCK_HEADER_SIZE);
}

void kfree(void* ptr)
{
    if (!ptr) return;
    
    HeapBlock* block = (HeapBlock*)((char*)ptr - HEAP_BLOCK_HEADER_SIZE);
    block->free = FREE;

    mergeBlocks(&kernelHeap);
}

void* umalloc(unsigned long size)
{
    if (size <= 0) return NULL;
    unsigned long totalSize = alignUpBlock(size + HEAP_BLOCK_HEADER_SIZE);
    HeapBlock* block = findFreeBlock(&userHeap, totalSize);
    if (!block)
    {
        print("kmalloc failed: out of memory (requested ", RED);
        printNumber(size, RED);
        printLine(" bytes)", RED);
        return NULL;
    }
    splitBlock(block, totalSize);
    block->free = OCCUPIED;

    return (void*)((char*)block + HEAP_BLOCK_HEADER_SIZE);
}

void ufree(void* ptr)
{
    if (!ptr) return;
    
    HeapBlock* block = (HeapBlock*)((char*)ptr - HEAP_BLOCK_HEADER_SIZE);
    block->free = FREE;
    
    mergeBlocks(&userHeap);
}

void initKernelHeap(void)
{
    kernelHeap.start = (void*)VIRTUAL_HEAP_START_ADDRESS;
    kernelHeap.end = (void*)VIRTUAL_HEAP_END_ADDRESS;
    kernelHeap.size = VIRTUAL_HEAP_END_ADDRESS - VIRTUAL_HEAP_START_ADDRESS;

    //its like a linked list(linked list allocator)
    kernelHeap.firstBlock = (HeapBlock*)kernelHeap.start;
    kernelHeap.firstBlock->size = kernelHeap.size;
    kernelHeap.firstBlock->free = FREE;
    kernelHeap.firstBlock->next = NULL;
}

void initUserHeap(void)
{
    userHeap.start = (void*)USER_HEAP_START;
    userHeap.end = (void*)USER_HEAP_END;
    userHeap.size = USER_HEAP_END - USER_HEAP_START;

    userHeap.firstBlock = (HeapBlock*)userHeap.start;
    userHeap.firstBlock->size = userHeap.size;
    userHeap.firstBlock->free = FREE;
    userHeap.firstBlock->next = NULL;

    unsigned long long physFrame = allocateFreeFrame();
    if (physFrame == ERROR) kernelPanic("Cannot allocate user heap!");
    //first page accessable
    memset((void*)(unsigned int)physFrame, 0, PAGE_SIZE);
    mapPage(USER_HEAP_START, (unsigned int)physFrame, 0);

}