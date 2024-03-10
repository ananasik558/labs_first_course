#ifndef __TEST_FUNCTIONS__
#define __TEST_FUNCTIONS__

#include "../include/buddySystemAllocator.h"

size_t getMicrotime();

// Тестирование на данных меньших, чем 16 байт. Последовательное выделение и освобождение
void testWithLightDataSequential(Allocator*);

// Тестирование на выделение с одновременным освобождением данные до 16 байт
void testLightDataRandom(Allocator*);

// Тестирование на данных от 16, до 256 байт. Последовательное выделение и освобождение
void testWithMediumDataSequential(Allocator*);

// Тестирование на выделение с одновременным освобождением на данных от 16 до 256 байт
void testWithMediumDataRandom(Allocator*);

// Тестирование на данных от 256, до 2048 байт. Последовательное выделение и освобождение
void testWithBigDataSequential(Allocator*);

// Тестирование на выделение с одновременным освобождением на данных от 256, до 2048 байт
void testWithBigDataRandom(Allocator*);


#endif // __TEST_FUNCTIONS__