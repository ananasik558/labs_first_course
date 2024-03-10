#ifndef __RESOURCE_MAP_ALLOCATOR__
#define __RESOURCE_MAP_ALLOCATOR__

#include <stddef.h>

#include "resourceMapBlockInfo.h"

#define ALIGN_BY (sizeof(BlockInfo))

typedef struct _Allocator {
    void* memory;
    size_t memorySize;
    BlockInfo* firstFreeBLock;
} Allocator;

size_t align(size_t);

Allocator* createMemoryAllocator(size_t);
void destroyMemoryAllocator(Allocator*);

void* allocBlock(Allocator*, size_t);
void freeBlock(Allocator*, void*);
size_t getBlockLengthByGivenMemory(void*);
void concatenateBlocks(Allocator*);

size_t getOffset(Allocator*, BlockInfo*);

#endif // __RESOURCE_MAP_ALLOCATOR__