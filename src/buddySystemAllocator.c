#include "../include/buddySystemAllocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/buddySystemLinkedList.h"

BlockInfo* blocks[MAX_POWER_OF_TWO - MIN_POWER_OF_TWO + 1];

void printBlocks(Allocator* allocator) {
    for (int i = 0; i < 10; ++i) {
        printf("I = %d\n", i);
        BlockInfo* currBlock = blocks[i];
        printf("Block size = %u\n", currBlock ? currBlock->size : 0);
        while (currBlock != NULL) {
            printf(" (%zu) ", (uint8_t*)currBlock - (uint8_t*)allocator->memory);
            currBlock = currBlock->next;
        }
        printf("\n");
        
    }
}

size_t align(size_t memorySize) {
    size_t currentSize = 1;
    while (currentSize < memorySize) {
        currentSize <<= 1;
    }
    return currentSize;
}

size_t getPowerOf2(size_t value) {
    size_t currentPower = 0;
    size_t currentValue = 1;
    while (currentValue < value) {
        currentValue <<= 1;
        ++currentPower;
    }
    return currentPower;
}

void initArrayOfBlocks() {
    for (size_t i = 0; i <= MAX_POWER_OF_TWO - MIN_POWER_OF_TWO; ++i) {
        blocks[i] = NULL;
    }
}

Allocator* createMemoryAllocator(size_t memorySize) {
    if (memorySize == 0) {
        fprintf(stderr, "Invalid memory size 0\n");
        exit(1);
    }

    Allocator* allocator = malloc(sizeof(Allocator));
    if (allocator == NULL) {
        perror("Can't create allocator");
        exit(1);
    }

    memorySize += sizeof(BlockInfo); // Добавляем размер структуры
    memorySize = align(memorySize); // Выравниваем саму память
    

    uint8_t* memory = malloc(memorySize); // malloc выравнивает память по 8 (или 16 или 32...)
    if (memory == NULL) {
        fprintf(stderr, "Can't allocate memory\n");
        exit(1);
    }
    allocator->memory = (void*)memory;
    allocator->memorySize = memorySize;

    BlockInfo block = {
        memorySize,
        true,
        NULL,
        NULL,
    };

    BlockInfo* firstBlock = (BlockInfo*)allocator->memory;
    *firstBlock = block;
    // Кладем в массив указателей первый, наибольший блок
    initArrayOfBlocks();
    blocks[getPowerOf2(memorySize) - MIN_POWER_OF_TWO] = firstBlock;
    return allocator;
}

void* allocBlock(Allocator* allocator, size_t blockSize) {
    // Число байт, которое необходимо, с учетом того, что входит еще и структура
    blockSize += sizeof(BlockInfo);
    blockSize = align(blockSize);

    size_t powerOf2 = getPowerOf2(blockSize);
    if (powerOf2 > MAX_POWER_OF_TWO) {
        fprintf(stderr, "Can't allocate memory\n");
        exit(1);
    }

    // Проверим, есть ли выше блок, из которого можно выделить нужное количество памяти
    size_t firstGoodBlockID = -1;
    
    for (size_t i = powerOf2 - MIN_POWER_OF_TWO; i <= MAX_POWER_OF_TWO - MIN_POWER_OF_TWO; ++i) {
        if (blocks[i] != NULL) {
            firstGoodBlockID = i;
            break;
        }
    }

    if (firstGoodBlockID == (size_t)-1) {
        
        #ifdef INFO
        for (int i = 0; i <= MAX_POWER_OF_TWO - MIN_POWER_OF_TWO; ++i) {
            printf("I = %d: P = %p\n", i, (void*)blocks[i]);
        }
        #endif

        fprintf(stderr, "Can't allocate memory\n");
        exit(1);
    }

    return recursiveAlloc(allocator, powerOf2, firstGoodBlockID, 1 << (firstGoodBlockID + MIN_POWER_OF_TWO));
}

void* recursiveAlloc(Allocator* allocator, size_t powerOfTwo, size_t goodBlockId, size_t blockSize) {
    BlockInfo* block = pop(&blocks[goodBlockId]);
    if (powerOfTwo == goodBlockId + MIN_POWER_OF_TWO) {
        block->isFree = false;
        return (uint8_t*)block + sizeof(BlockInfo);
    }

    size_t newBlockSize = blockSize / 2;
    size_t newBlockID = goodBlockId - 1;
    block->size = newBlockSize;
    BlockInfo secondBlock = {
        newBlockSize,
        true,
        NULL,
        NULL,
    };

    // Находим место в память для блока близнеца, кладем туда новый блок
    BlockInfo* secondBlockPtr = (BlockInfo*)(((uint8_t*)block) + newBlockSize);
    *secondBlockPtr = secondBlock;
    push(&blocks[newBlockID], secondBlockPtr);
    push(&blocks[newBlockID], block);

    return recursiveAlloc(allocator, powerOfTwo, newBlockID, newBlockSize);
}

void freeBlock(const Allocator* allocator, void* block) {
    BlockInfo* currentBlock = (BlockInfo*)(((uint8_t*)block) - sizeof(BlockInfo));
    if (currentBlock->isFree) {
        fprintf(stderr, "This block already free, can't free again\n");
        exit(1);
    }
    currentBlock->isFree = true;

    concatenateBlocks(allocator, currentBlock);
}

void concatenateBlocks(const Allocator* allocator, BlockInfo* currentBlock) {
    // printf("Start concat\n");
    BlockInfo* buddy = getBuddy(allocator, currentBlock);

    size_t blockID = getPowerOf2(currentBlock->size) - MIN_POWER_OF_TWO;
    if (currentBlock < buddy) {
        if (buddy->isFree && buddy->size == currentBlock->size) {
            // printf("HERE1\n");
            removeBlock(&blocks[blockID], buddy);
            currentBlock->size *= 2;
            concatenateBlocks(allocator, currentBlock);
        } else {
            push(&blocks[blockID], currentBlock);
        }
    } else if (currentBlock > buddy) {
        if (buddy->isFree && buddy->size == currentBlock->size) {
            // printf("HERE2\n");
            removeBlock(&blocks[blockID], buddy);
            // printf("HERE22\n");
            buddy->size *= 2;
            concatenateBlocks(allocator, buddy);
        } else {
            push(&blocks[blockID], currentBlock);
        }
    } else {
        fprintf(stderr, "Something went wrong ...\n");
        exit(1);
    }
    // printf("End concat\n");
}

BlockInfo* getBuddy(const Allocator* allocator, BlockInfo* currentBlock) {
    return (BlockInfo*)((uint8_t*)allocator->memory + (((size_t)(((uint8_t*)currentBlock) - ((uint8_t*)allocator->memory))) ^ ((size_t)currentBlock->size)));
}

void destroyMemoryAllocator(Allocator* allocator) {
    free(allocator->memory);
    free(allocator);
}