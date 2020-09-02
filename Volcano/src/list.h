#pragma once
#include <stdio.h>

#include "errorCatch.h"
typedef struct List{
    struct List* previous;
    struct List* next;
    void* content;
}List;
List* NewList(void* content);
unsigned int GetListCount(List* list);
List* GetElement(List* list,unsigned int index);
void AddElementAfterElement(List* element, void* content);
void AddElementBeforeElement(List* element, void* content);
//pushes element at that index forward 1
void AddElementAtIndex(List* list, unsigned int index, void* content);
void AddElementAtEnd(List* list, void* content);
void RemoveLastElement(List* list);
void RemoveElement(List* list,unsigned int index);
void RemoveElementByContent(List* list, void* content);