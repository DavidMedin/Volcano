#pragma once
#include <stdio.h>

#include "errorCatch.h"
typedef struct List{
    List* previous;
    List* next;
    void* content;
}List;
void NewList(List** list,void* content);
void AddElementAfterElement(List* element, void* content);
void AddElementBeforeElement(List* list,List* element, void* content);
//pushes element at that index forward 1
void AddElementAtIndex(List* list, unsigned int index, void* content);
void AddElementAtEnd(List* list, void* content);
void RemoveElement(List* list);
void RemoveElement(List* list, void* content);
void RemoveElement(List* list,unsigned int index);