#include "../include/buddySystemLinkedList.h"

#include <stdio.h>

void push(BlockInfo** head, BlockInfo* new) {
    new->next = *head;
    new->prev = NULL;

    if (*head != NULL) {
        (*head)->prev = new;
    }

    *head = new;

}

BlockInfo* pop(BlockInfo** head) {
    BlockInfo* result;
    result = *head;
    if ((*head)->next == NULL) {
        *head = NULL;
    } else {
        (*head)->next->prev = NULL;
        (*head) = (*head)->next;
        result->next = NULL;
    }
    return result;
}

void removeBlock(BlockInfo** head, BlockInfo* target) {
    // Удаление с начала
    if ((*head) == target) {
        BlockInfo* tmp = (*head);
        *head = (*head)->next;
        if (*head != NULL) {
            (*head)->prev = NULL;
        }
        tmp->next = NULL;
        return;
    }
    // Удаление с конца
    if (target->next == NULL) {
        target->prev->next = NULL;
        target->prev = NULL;
        return;
    }
    BlockInfo* currentBlock = target;
    currentBlock->prev->next = currentBlock->next;
    currentBlock->next->prev = currentBlock->prev;
    currentBlock->next = NULL;
    currentBlock->prev = NULL; 
}