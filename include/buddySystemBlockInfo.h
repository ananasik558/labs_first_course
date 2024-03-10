#ifndef __BUDDY_SYSTEM_BLOCK_INFO__
#define __BUDDY_SYSTEM_BLOCK_INFO__

#include <stdint.h>

typedef struct _BlockInfo {
    uint32_t size;
    uint32_t isFree;
    struct _BlockInfo* next;
    struct _BlockInfo* prev;
} BlockInfo;

#endif // __BUDDY_SYSTEM_BLOCK_INFO__