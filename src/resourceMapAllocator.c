#include "../include/resourceMapAllocator.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "../include/resourceMapBlockInfo.h"

size_t align(size_t memorySize) {
    return memorySize + (ALIGN_BY - (memorySize % ALIGN_BY)) % ALIGN_BY;
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

    // Добавляем размер структуры
    memorySize += ALIGN_BY;
    memorySize = align(memorySize); // Выравниваем саму память
    uint8_t* memory = malloc(memorySize); // malloc выравнивает память по 8 (или 16 или 32...)

    #ifdef INFO
    printf("ALLOCATED TOTAL %zu\n", memorySize);
    #endif

    if (memory == NULL) {
        fprintf(stderr, "Can't allocate memory\n");
        exit(1);
    }

    allocator->memory = (void*)memory;
    allocator->memorySize = memorySize;

    // В начало ставим блок, который указывает на всю память
    BlockInfo block = {
        NULL,
        memorySize - ALIGN_BY,
    };
    BlockInfo* firstBlock = (BlockInfo*)allocator->memory;
    *firstBlock = block;

    setBlockFree(firstBlock);
    allocator->firstFreeBLock = firstBlock;
    return allocator;
}

size_t getBlockLengthByGivenMemory(void* memory) {
    return ((BlockInfo*)((uint8_t*)memory - ALIGN_BY))->blockSize;
}

// Сдвиг начала блока относительно начала выделенной памяти
size_t getOffset(Allocator* allocator, BlockInfo* block) {
    return (size_t)((uint8_t*)block - (uint8_t*)allocator->memory);
}

void* allocBlock(Allocator* allocator, size_t requestedMemory) {
    requestedMemory = align(requestedMemory);
    BlockInfo* currentBlock = allocator->firstFreeBLock;
    BlockInfo* prevBlock = NULL;
    BlockInfo* bestBlock = NULL;
    BlockInfo* bestPrevBlock = NULL;
    size_t minLength = -1;

    while (currentBlock != NULL) {
        size_t currentBlockSize = currentBlock->blockSize;
        if (currentBlockBetter(currentBlockSize, requestedMemory, minLength)) {
            bestBlock = currentBlock;
            bestPrevBlock = prevBlock;
            minLength = currentBlockSize;
        }
        prevBlock = currentBlock;
        currentBlock = resetToNormalPointer(currentBlock->nextBlock);
    }

    if (bestBlock == NULL) {
        fprintf(stderr, "Can't allocate new memory\n");
        exit(1);
    }

    // Можно выделить память еще для следующего блока, он будет свободным
    if (requestedMemory + sizeof(BlockInfo) <= bestBlock->blockSize) {
        // Расположение нового блока
        BlockInfo* newBlock = (BlockInfo*)((uint8_t*)bestBlock + sizeof(BlockInfo) + requestedMemory);
        newBlock->nextBlock = bestBlock->nextBlock;
        newBlock->blockSize = bestBlock->blockSize - requestedMemory - sizeof(BlockInfo);
        setBlockFree(newBlock);

        // Меняем блок, из которого была выделена памяти
        setBlockOccupied(bestBlock);
        bestBlock->blockSize -= (sizeof(BlockInfo) + newBlock->blockSize);

        // Удаляем этот блок из списка
        if (bestPrevBlock == NULL) {
            allocator->firstFreeBLock = newBlock;
        } else {
            bestPrevBlock->nextBlock = newBlock;
        }
    } else { // не хватило памяти под следующий блок
        setBlockOccupied(bestBlock);
        if (bestPrevBlock == NULL) {
            allocator->firstFreeBLock = bestBlock->nextBlock;
        } else {
            bestPrevBlock->nextBlock = bestBlock->nextBlock;
        }
    }

    return (void*)(((uint8_t*)bestBlock) + ALIGN_BY);
}

void freeBlock(Allocator* allocator, void* memoryBlock) {
    BlockInfo* blockForFree = (BlockInfo*)((uint8_t*)memoryBlock - ALIGN_BY);
    if (!(allocator->memory <= memoryBlock && memoryBlock <= (void*)((uint8_t*)allocator->memory + allocator->memorySize)) || isBlockFree(blockForFree)) {
        fprintf(stderr, "Can't free this block\n");
        exit(1);
    }

    // Будем проходится по все блокам, до тех пор, пока не найдем, куда поставить нвоый блок
    BlockInfo* currentBlock = allocator->firstFreeBLock;

    if (currentBlock == blockForFree) {
        fprintf(stderr, "Can't free this block\n");
        exit(1);
    }

    // Надо добавить блок в самое начало
    if (blockForFree < currentBlock || currentBlock == NULL) {
        blockForFree->nextBlock = currentBlock;
        setBlockFree(blockForFree);
        allocator->firstFreeBLock = blockForFree;
    } else { // Блок будет находиться не в начале
        while (currentBlock->nextBlock < blockForFree && resetToNormalPointer(currentBlock->nextBlock) != NULL) {
            currentBlock = resetToNormalPointer(currentBlock->nextBlock);
        }
        blockForFree->nextBlock = resetToNormalPointer(currentBlock->nextBlock);
        currentBlock->nextBlock = blockForFree;
        setBlockFree(blockForFree);
        setBlockFree(currentBlock);
    }

    currentBlock = allocator->firstFreeBLock;
    while (currentBlock != NULL) {
        if (canConcatenate(currentBlock)) {
            currentBlock->blockSize += ALIGN_BY + resetToNormalPointer(currentBlock->nextBlock)->blockSize;
            currentBlock->nextBlock = resetToNormalPointer(currentBlock->nextBlock)->nextBlock;
            continue;
        }
        currentBlock = resetToNormalPointer(currentBlock->nextBlock);
    }
    
}

bool canConcatenate(BlockInfo* currentBlock) {
    return resetToNormalPointer(currentBlock->nextBlock) == (BlockInfo*)((uint8_t*)currentBlock + ALIGN_BY + currentBlock->blockSize);
}

bool currentBlockBetter(size_t currentBlockLength, size_t requestedMemory, size_t minLength) {
    return currentBlockLength >= requestedMemory && currentBlockLength < minLength;
}

void destroyMemoryAllocator(Allocator* allocator) {
    free(allocator->memory);
    free(allocator);
}