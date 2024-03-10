#include "../include/testFunctionsBSAllocator.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

#include "../include/testSettings.h"
#include "../include/buddySystemAllocator.h"

size_t getMicrotime() {
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return (size_t)currentTime.tv_sec * (size_t)1e6 + (size_t)currentTime.tv_usec;
}

void testWithLightDataSequential(Allocator* allocator) {
    printf("-----------------------------------------------------------------------------\n");
    printf("Testing on data betwen 1 and 16 bytes. Sequential allocation and release\nTotal number of tests: %d\n\n", LIGHT_DATA_SEQUENTIAL_TEST_COUNT);
    srand(time(NULL));

    size_t totalRequested = 0;
    size_t totalAllocated = 0;
    size_t totalTimeToAllocate = 0;
    size_t totalTimeToFree = 0;

    size_t testVariable = 0;

    char** arrays = allocBlock(allocator, sizeof(char*) * LIGHT_DATA_SEQUENTIAL_ARRAY_SIZE);

    for (size_t iteration = 0; iteration < LIGHT_DATA_SEQUENTIAL_TEST_COUNT; ++iteration) {

        #ifdef INFO
        printf("Test number: %zu\n", iteration + 1);
        #endif

        size_t currentIterationStart = getMicrotime();

        // Выделяем
        for (size_t i = 0; i < LIGHT_DATA_SEQUENTIAL_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Allocating. I = %zu\n", i);
            #endif

            size_t currentSize = sizeof(char) * ((rand() % 16) + 1);
            arrays[i] = allocBlock(allocator, currentSize);

            arrays[i][0] = (i + currentSize) % 10; // Просто заполняем, чтобы компилятор ничего не оптимизировал
            testVariable += arrays[i][0];

            totalRequested += currentSize;
            totalAllocated += ((BlockInfo*)((uint8_t*)arrays[i] - sizeof(BlockInfo)))->size;
        }

        // Добавляем время необходимое для выделения памяти
        totalTimeToAllocate += (getMicrotime() - currentIterationStart);

        size_t currentFreeStart = getMicrotime();

        for (size_t i = 0; i < LIGHT_DATA_SEQUENTIAL_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Free. I = %zu\n", i);
            #endif

            freeBlock(allocator, arrays[i]);
        }

        totalTimeToFree += (getMicrotime() - currentFreeStart);
    }

    printf("Total memory requested: %zu\n", totalRequested);
    printf("Total memory allocated: %zu\n", totalAllocated);
    printf("Microseconds to allocate: %zu\n", totalTimeToAllocate);
    printf("Microseconds to free: %zu\n\n", totalTimeToFree);

    printf("Average requested memory: %Lf\n", (long double)totalRequested / LIGHT_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average allocated memory: %Lf\n", (long double)totalAllocated / LIGHT_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average microseconds to allocate: %Lf\n", (long double)totalTimeToAllocate / LIGHT_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average microseconds to free: %Lf\n", (long double)totalTimeToFree / LIGHT_DATA_SEQUENTIAL_TEST_COUNT);
    printf("%zu\n", testVariable);
    printf("-----------------------------------------------------------------------------\n\n");

    freeBlock(allocator, arrays);
}

void testLightDataRandom(Allocator* allocator) {
    printf("-----------------------------------------------------------------------------\n");
    printf("Testing on data betwen 1 and 16 bytes. With probability 1/8 5 random elements are removed.\nTotal number of tests: %d\n\n", LIGHT_DATA_RANDOM_TEST_COUNT);
    srand(time(NULL));

    size_t totalRequested = 0;
    size_t totalAllocated = 0;
    size_t totalTimeToAllocate = 0;
    size_t totalTimeToFree = 0;

    size_t testVariable = 0;

    char** arrays = allocBlock(allocator, sizeof(char*) * LIGHT_DATA_RANDOM_ARRAY_SIZE);
    for (size_t i = 0; i < LIGHT_DATA_RANDOM_ARRAY_SIZE; ++i) {
        arrays[i] = NULL;
    }

    for (size_t iteration = 0; iteration < LIGHT_DATA_RANDOM_TEST_COUNT; ++iteration) {

        #ifdef INFO
        printf("Iteration = %zu\n", iteration);
        #endif

        size_t startAllocatingTime = getMicrotime();
        size_t timeToFreeWhileAllocating = 0;

        // Выделяем
        for (size_t i = 0; i < LIGHT_DATA_RANDOM_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Allocating. I = %zu\n", i);
            #endif

            size_t currentSize = sizeof(char) * ((rand() % 16) + 1);
            arrays[i] = allocBlock(allocator, currentSize);

            arrays[i][0] = (i + currentSize) % 10; // Просто заполняем, чтобы компилятор ничего не оптимизировал
            testVariable += arrays[i][0];

            totalRequested += currentSize;
            totalAllocated += ((BlockInfo*)((uint8_t*)arrays[i] - sizeof(BlockInfo)))->size;

            // С вероятностью 1 / 8 будем очищать 5 элементов
            if (currentSize <= 2) {

                size_t timeForFreeRandom = getMicrotime();

                for (size_t j = 0; j < 5; ++j) {
                    size_t indexToDelete = rand() % (i + 1);
                    if (arrays[indexToDelete]) {
                        
                        #ifdef INFO
                        printf("Free. I = %zu\n", indexToDelete);
                        #endif

                        freeBlock(allocator, arrays[indexToDelete]);
                        arrays[indexToDelete] = NULL;
                    }
                }

                size_t currentMicroseconds = getMicrotime();

                timeToFreeWhileAllocating += (currentMicroseconds - timeForFreeRandom);
                totalTimeToFree += (currentMicroseconds - timeForFreeRandom);
            }
        }

        totalTimeToAllocate += (getMicrotime() - startAllocatingTime - timeToFreeWhileAllocating);

        size_t startFreeTime = getMicrotime();

        // Освобождаем остатки памяти
        for (size_t i = 0; i < LIGHT_DATA_RANDOM_ARRAY_SIZE; ++i) {
            if (arrays[i]) {

                #ifdef INFO
                printf("Free. I = %zu\n", i);
                #endif
                
                freeBlock(allocator, arrays[i]);
                arrays[i] = NULL;
            }
        }

        totalTimeToFree += (getMicrotime() - startFreeTime);
    }

    printf("Total memory requested: %zu\n", totalRequested);
    printf("Total memory allocated: %zu\n", totalAllocated);
    printf("Microseconds to allocate: %zu\n", totalTimeToAllocate);
    printf("Microseconds to free: %zu\n\n", totalTimeToFree);

    printf("Average requested memory: %Lf\n", (long double)totalRequested / LIGHT_DATA_RANDOM_TEST_COUNT);
    printf("Average allocated memory: %Lf\n", (long double)totalAllocated / LIGHT_DATA_RANDOM_TEST_COUNT);
    printf("Average microseconds to allocate: %Lf\n", (long double)totalTimeToAllocate / LIGHT_DATA_RANDOM_TEST_COUNT);
    printf("Average microseconds to free: %Lf\n", (long double)totalTimeToFree / LIGHT_DATA_RANDOM_TEST_COUNT);
    printf("%zu\n", testVariable);
    printf("-----------------------------------------------------------------------------\n\n");

    freeBlock(allocator, arrays);
}

void testWithMediumDataSequential(Allocator* allocator) {
    printf("-----------------------------------------------------------------------------\n");
    printf("Testing on data betwen 16 and 256 bytes. Sequential allocation and release\nTotal number of tests: %d\n\n", MEDIUM_DATA_SEQUENTIAL_TEST_COUNT);
    srand(time(NULL));

    size_t totalRequested = 0;
    size_t totalAllocated = 0;
    size_t totalTimeToAllocate = 0;
    size_t totalTimeToFree = 0;

    size_t testVariable = 0;

    char** arrays = allocBlock(allocator, sizeof(char*) * MEDIUM_DATA_SEQUENTIAL_ARRAY_SIZE);

    for (size_t iteration = 0; iteration < MEDIUM_DATA_SEQUENTIAL_TEST_COUNT; ++iteration) {

        #ifdef INFO
        printf("Test number: %zu\n", iteration + 1);
        #endif

        size_t currentIterationStart = getMicrotime();

        // Выделяем
        for (size_t i = 0; i < MEDIUM_DATA_SEQUENTIAL_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Allocating. I = %zu\n", i);
            #endif

            // Размер будет от 16 до 256 байт
            size_t currentSize = sizeof(char) * ((rand() % (256 - 15)) + 16);
            arrays[i] = allocBlock(allocator, currentSize);

            arrays[i][0] = (i + currentSize) % 10; // Просто заполняем, чтобы компилятор ничего не оптимизировал
            testVariable += arrays[i][0];

            totalRequested += currentSize;
            totalAllocated += ((BlockInfo*)((uint8_t*)arrays[i] - sizeof(BlockInfo)))->size;
        }

        // Добавляем время необходимое для выделения памяти
        totalTimeToAllocate += (getMicrotime() - currentIterationStart);

        size_t currentFreeStart = getMicrotime();

        for (size_t i = 0; i < MEDIUM_DATA_SEQUENTIAL_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Free. I = %zu\n", i);
            #endif

            freeBlock(allocator, arrays[i]);
        }

        totalTimeToFree += (getMicrotime() - currentFreeStart);
    }

    printf("Total memory requested: %zu\n", totalRequested);
    printf("Total memory allocated: %zu\n", totalAllocated);
    printf("Microseconds to allocate: %zu\n", totalTimeToAllocate);
    printf("Microseconds to free: %zu\n\n", totalTimeToFree);

    printf("Average requested memory: %Lf\n", (long double)totalRequested / MEDIUM_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average allocated memory: %Lf\n", (long double)totalAllocated / MEDIUM_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average microseconds to allocate: %Lf\n", (long double)totalTimeToAllocate / MEDIUM_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average microseconds to free: %Lf\n", (long double)totalTimeToFree / MEDIUM_DATA_SEQUENTIAL_TEST_COUNT);
    printf("%zu\n", testVariable);
    printf("-----------------------------------------------------------------------------\n\n");

    freeBlock(allocator, arrays);
}

void testWithMediumDataRandom(Allocator* allocator) {
    printf("-----------------------------------------------------------------------------\n");
    printf("Testing on data betwen 16 and 256 bytes. With probability 1/8 5 random elements are removed.\nTotal number of tests: %d\n\n", MEDIUM_DATA_RANDOM_TEST_COUNT);
    srand(time(NULL));

    size_t totalRequested = 0;
    size_t totalAllocated = 0;
    size_t totalTimeToAllocate = 0;
    size_t totalTimeToFree = 0;

    size_t testVariable = 0;

    char** arrays = allocBlock(allocator, sizeof(char*) * MEDIUM_DATA_RANDOM_ARRAY_SIZE);
    for (size_t i = 0; i < MEDIUM_DATA_RANDOM_ARRAY_SIZE; ++i) {
        arrays[i] = NULL;
    }

    for (size_t iteration = 0; iteration < MEDIUM_DATA_RANDOM_TEST_COUNT; ++iteration) {

        #ifdef INFO
        printf("Iteration = %zu\n", iteration);
        #endif

        size_t startAllocatingTime = getMicrotime();
        size_t timeToFreeWhileAllocating = 0;

        // Выделяем
        for (size_t i = 0; i < MEDIUM_DATA_RANDOM_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Allocating. I = %zu\n", i);
            #endif

            size_t currentSize = sizeof(char) * ((rand() % (256 - 15)) + 16);
            arrays[i] = allocBlock(allocator, currentSize);

            arrays[i][0] = (i + currentSize) % 10; // Просто заполняем, чтобы компилятор ничего не оптимизировал
            testVariable += arrays[i][0];

            totalRequested += currentSize;
            totalAllocated += ((BlockInfo*)((uint8_t*)arrays[i] - sizeof(BlockInfo)))->size;

            // С вероятностью 1 / 8 будем очищать 5 элементов
            if (currentSize <= 46) {

                size_t timeForFreeRandom = getMicrotime();

                for (size_t j = 0; j < 5; ++j) {
                    size_t indexToDelete = rand() % (i + 1);
                    if (arrays[indexToDelete]) {
                        
                        #ifdef INFO
                        printf("Free. I = %zu\n", indexToDelete);
                        #endif

                        freeBlock(allocator, arrays[indexToDelete]);
                        arrays[indexToDelete] = NULL;
                    }
                }

                size_t currentMicroseconds = getMicrotime();

                timeToFreeWhileAllocating += (currentMicroseconds - timeForFreeRandom);
                totalTimeToFree += (currentMicroseconds - timeForFreeRandom);
            }
        }

        totalTimeToAllocate += (getMicrotime() - startAllocatingTime - timeToFreeWhileAllocating);

        size_t startFreeTime = getMicrotime();

        // Освобождаем остатки памяти
        for (size_t i = 0; i < MEDIUM_DATA_RANDOM_ARRAY_SIZE; ++i) {
            if (arrays[i]) {

                #ifdef INFO
                printf("Free. I = %zu\n", i);
                #endif
                
                freeBlock(allocator, arrays[i]);
                arrays[i] = NULL;
            }
        }

        totalTimeToFree += (getMicrotime() - startFreeTime);
    }

    printf("Total memory requested: %zu\n", totalRequested);
    printf("Total memory allocated: %zu\n", totalAllocated);
    printf("Microseconds to allocate: %zu\n", totalTimeToAllocate);
    printf("Microseconds to free: %zu\n\n", totalTimeToFree);

    printf("Average requested memory: %Lf\n", (long double)totalRequested / MEDIUM_DATA_RANDOM_TEST_COUNT);
    printf("Average allocated memory: %Lf\n", (long double)totalAllocated / MEDIUM_DATA_RANDOM_TEST_COUNT);
    printf("Average microseconds to allocate: %Lf\n", (long double)totalTimeToAllocate / MEDIUM_DATA_RANDOM_TEST_COUNT);
    printf("Average microseconds to free: %Lf\n", (long double)totalTimeToFree / MEDIUM_DATA_RANDOM_TEST_COUNT);
    printf("%zu\n", testVariable);
    printf("-----------------------------------------------------------------------------\n\n");

    freeBlock(allocator, arrays);
}

void testWithBigDataSequential(Allocator* allocator) {
    printf("-----------------------------------------------------------------------------\n");
    printf("Testing on data betwen 256 and 2048 bytes. Sequential allocation and release\nTotal number of tests: %d\n\n", BIG_DATA_SEQUENTIAL_TEST_COUNT);
    srand(time(NULL));

    size_t totalRequested = 0;
    size_t totalAllocated = 0;
    size_t totalTimeToAllocate = 0;
    size_t totalTimeToFree = 0;

    size_t testVariable = 0;

    char** arrays = allocBlock(allocator, sizeof(char*) * BIG_DATA_SEQUENTIAL_ARRAY_SIZE);

    for (size_t iteration = 0; iteration < BIG_DATA_SEQUENTIAL_TEST_COUNT; ++iteration) {

        #ifdef INFO
        printf("Test number: %zu\n", iteration + 1);
        #endif

        size_t currentIterationStart = getMicrotime();

        // Выделяем
        for (size_t i = 0; i < BIG_DATA_SEQUENTIAL_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Allocating. I = %zu\n", i);
            #endif

            // Размер будет от 256 до 2048 байт
            size_t currentSize = sizeof(char) * ((rand() % (2048 - 255)) + 256);
            arrays[i] = allocBlock(allocator, currentSize);

            arrays[i][0] = (i + currentSize) % 10; // Просто заполняем, чтобы компилятор ничего не оптимизировал
            testVariable += arrays[i][0];

            totalRequested += currentSize;
            totalAllocated += ((BlockInfo*)((uint8_t*)arrays[i] - sizeof(BlockInfo)))->size;

            #ifdef INFO
            printf("Total allocated: %zu\n", totalAllocated);
            #endif
        }

        // Добавляем время необходимое для выделения памяти
        totalTimeToAllocate += (getMicrotime() - currentIterationStart);

        size_t currentFreeStart = getMicrotime();

        for (size_t i = 0; i < BIG_DATA_SEQUENTIAL_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Free. I = %zu\n", i);
            #endif

            freeBlock(allocator, arrays[i]);
        }

        totalTimeToFree += (getMicrotime() - currentFreeStart);
    }

    printf("Total memory requested: %zu\n", totalRequested);
    printf("Total memory allocated: %zu\n", totalAllocated);
    printf("Microseconds to allocate: %zu\n", totalTimeToAllocate);
    printf("Microseconds to free: %zu\n\n", totalTimeToFree);

    printf("Average requested memory: %Lf\n", (long double)totalRequested / BIG_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average allocated memory: %Lf\n", (long double)totalAllocated / BIG_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average microseconds to allocate: %Lf\n", (long double)totalTimeToAllocate / BIG_DATA_SEQUENTIAL_TEST_COUNT);
    printf("Average microseconds to free: %Lf\n", (long double)totalTimeToFree / BIG_DATA_SEQUENTIAL_TEST_COUNT);
    printf("%zu\n", testVariable);
    printf("-----------------------------------------------------------------------------\n\n");

    freeBlock(allocator, arrays);
}

void testWithBigDataRandom(Allocator* allocator) {
    printf("-----------------------------------------------------------------------------\n");
    printf("Testing on data betwen 256 and 2048 bytes. With probability 1/8 5 random elements are removed.\nTotal number of tests: %d\n\n", BIG_DATA_RANDOM_TEST_COUNT);
    srand(time(NULL));

    size_t totalRequested = 0;
    size_t totalAllocated = 0;
    size_t totalTimeToAllocate = 0;
    size_t totalTimeToFree = 0;

    size_t testVariable = 0;

    char** arrays = allocBlock(allocator, sizeof(char*) * BIG_DATA_RANDOM_ARRAY_SIZE);
    for (size_t i = 0; i < BIG_DATA_RANDOM_ARRAY_SIZE; ++i) {
        arrays[i] = NULL;
    }

    for (size_t iteration = 0; iteration < MEDIUM_DATA_RANDOM_TEST_COUNT; ++iteration) {

        #ifdef INFO
        printf("Iteration = %zu\n", iteration);
        #endif

        size_t startAllocatingTime = getMicrotime();
        size_t timeToFreeWhileAllocating = 0;

        // Выделяем
        for (size_t i = 0; i < BIG_DATA_RANDOM_ARRAY_SIZE; ++i) {

            #ifdef INFO
            printf("Allocating. I = %zu\n", i);
            #endif

            size_t currentSize = sizeof(char) * ((rand() % (2048 - 255)) + 256);
            arrays[i] = allocBlock(allocator, currentSize);

            arrays[i][0] = (i + currentSize) % 10; // Просто заполняем, чтобы компилятор ничего не оптимизировал
            testVariable += arrays[i][0];

            totalRequested += currentSize;
            totalAllocated += ((BlockInfo*)((uint8_t*)arrays[i] - sizeof(BlockInfo)))->size;

            // С вероятностью 1 / 8 будем очищать 5 элементов
            if (currentSize <= 224 + 256) {

                size_t timeForFreeRandom = getMicrotime();

                for (size_t j = 0; j < 5; ++j) {
                    size_t indexToDelete = rand() % (i + 1);
                    if (arrays[indexToDelete]) {
                        
                        #ifdef INFO
                        printf("Free. I = %zu\n", indexToDelete);
                        #endif

                        freeBlock(allocator, arrays[indexToDelete]);
                        arrays[indexToDelete] = NULL;
                    }
                }

                size_t currentMicroseconds = getMicrotime();

                timeToFreeWhileAllocating += (currentMicroseconds - timeForFreeRandom);
                totalTimeToFree += (currentMicroseconds - timeForFreeRandom);
            }
        }

        totalTimeToAllocate += (getMicrotime() - startAllocatingTime - timeToFreeWhileAllocating);

        size_t startFreeTime = getMicrotime();

        // Освобождаем остатки памяти
        for (size_t i = 0; i < BIG_DATA_RANDOM_ARRAY_SIZE; ++i) {
            if (arrays[i]) {

                #ifdef INFO
                printf("Free. I = %zu\n", i);
                #endif
                
                freeBlock(allocator, arrays[i]);
                arrays[i] = NULL;
            }
        }

        totalTimeToFree += (getMicrotime() - startFreeTime);
    }

    printf("Total memory requested: %zu\n", totalRequested);
    printf("Total memory allocated: %zu\n", totalAllocated);
    printf("Microseconds to allocate: %zu\n", totalTimeToAllocate);
    printf("Microseconds to free: %zu\n\n", totalTimeToFree);

    printf("Average requested memory: %Lf\n", (long double)totalRequested / BIG_DATA_RANDOM_TEST_COUNT);
    printf("Average allocated memory: %Lf\n", (long double)totalAllocated / BIG_DATA_RANDOM_TEST_COUNT);
    printf("Average microseconds to allocate: %Lf\n", (long double)totalTimeToAllocate / BIG_DATA_RANDOM_TEST_COUNT);
    printf("Average microseconds to free: %Lf\n", (long double)totalTimeToFree / BIG_DATA_RANDOM_TEST_COUNT);
    printf("%zu\n", testVariable);
    printf("-----------------------------------------------------------------------------\n\n");

    freeBlock(allocator, arrays);
}