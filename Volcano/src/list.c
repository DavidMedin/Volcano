#include "list.h"
void NewList(List** list, void* content){
    *list = malloc(sizeof(List));
    (*list)->previous = NULL;
    (*list)->next = NULL;
    (*list)->content = content;
}
void AddElementAfterElement(List* element, void* content){
    List* newElement = malloc(sizeof(List));
    newElement->previous = element;
    newElement->next = element->next;
    element->next = newElement;
}
//in a list
void AddElementBeforeElement(List* element, void* content){
    List* newElement = malloc(sizeof(List));
    if(element->previous != NULL)
        AddElementAfterElement(element->previous,content);
    else{
        newElement->next = element;
        newElement->previous = NULL;
        element->previous = newElement;
    }

}
void AddElementAtIndex(List* list, unsigned int index, void* content){
    List* target = list;
    for(unsigned int i = 0;i < index;i++){
        if(target->next != NULL)
            Error("Index outside of range!\n");
        else
            target = target->next;
    }
    AddElementBeforeElement(list,target,content);
}
void AddElementAtEnd(List* list, void* content){
    List* newElement = malloc(sizeof(List));
    List* target = NULL;
    while(target->next != NULL)
        target = target->next;
    target->next = newElement;
    newElement->previous = target;
}
void RemoveElement(List* list);{

}
void RemoveElement(List* list, void* content);
void RemoveElement(List* list,unsigned int index);