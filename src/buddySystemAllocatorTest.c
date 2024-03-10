#include "../include/buddySystemAllocator.h"
#include "../include/testFunctionsBSAllocator.h"

int main() {

    // Testing on light data
    Allocator* allocatorLight = createMemoryAllocator(40000000);
    testWithLightDataSequential(allocatorLight);
    testLightDataRandom(allocatorLight);
    destroyMemoryAllocator(allocatorLight);

    // Testing on medium data
    Allocator* allocatorMedium = createMemoryAllocator(600000000);
    testWithMediumDataSequential(allocatorMedium);
    testWithMediumDataRandom(allocatorMedium);
    destroyMemoryAllocator(allocatorMedium);

    // Раньше у меня размеры всех блоков были одинаковые (по 32 байта, и следовательно первым подходящим был самый первый блок)
    // Теперь блоки имею разную длину, и следовательно довольно вероятно, что не найдется блока нужного размера и придется разместить новый болк в конце
    // Из-за этого увеличивается цепочка подряд идущих блоков => поиск дольше => освобождение дольше

    // Testing on big data
    Allocator* allocatorBig = createMemoryAllocator(1000000000);
    testWithBigDataSequential(allocatorBig);
    testWithBigDataRandom(allocatorBig);
    destroyMemoryAllocator(allocatorBig);

    // RM аллокатор работает быстрее на последовательных данных, т.к. там сразу известен первый свободный блок

}