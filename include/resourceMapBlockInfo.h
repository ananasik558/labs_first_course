#ifndef __RESOURCE_MAP_BLOCK_INFO__
#define __RESOURCE_MAP_BLOCK_INFO__

#include <stdlib.h>
#include <stdbool.h>

typedef struct _BlockInfo {
    struct _BlockInfo* nextBlock;
    size_t blockSize;
} BlockInfo;

bool isBlockFree(BlockInfo*);
BlockInfo* setBlockFree(BlockInfo*);
BlockInfo* setBlockOccupied(BlockInfo*);
BlockInfo* resetToNormalPointer(BlockInfo*);

bool canConcatenate(BlockInfo*);
bool currentBlockBetter(size_t, size_t, size_t);

#endif // __RESOURCE_MAP_BLOCK_INFO__