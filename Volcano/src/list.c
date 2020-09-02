#include "list.h"
List* NewList(void* content){
    List* list = malloc(sizeof(List));
    (list)->previous = NULL;
    (list)->next = NULL;
    (list)->content = content;
    return list;
}
unsigned int GetListCount(List* list){
    List* target = list;
    unsigned int count = 0;
    while(target->next != NULL){
        count++;
        target = target->next;
    }
    return count;
}
List* GetElement(List* list,unsigned int index){
    List* target = list;
    for(unsigned int i = 0;i < index;i++){
        if(target->next == NULL){
            Error("List: Get index out of range!\n");
            return NULL;
        }
        target = target->next;
    }
    return target;
}
void AddElementAfterElement(List* element, void* content){
    List* newElement = malloc(sizeof(List));
    newElement->previous = element;
    newElement->next = element->next;
    element->next = newElement;
    newElement->content = content;
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
        newElement->content = content;

    }

}
void AddElementAtIndex(List* list, unsigned int index, void* content){
    List* target = list;
    for(unsigned int i = 0;i < index;i++){
        if(target->next == NULL){
            Error("List: Index outside of range!\n");
        }else
            target = target->next;
    }
    AddElementBeforeElement(target,content);
}
void AddElementAtEnd(List* list, void* content){
    List* newElement = malloc(sizeof(List));
    List* target = list;
    while(target->next != NULL)
        target = target->next;
    target->next = newElement;
    newElement->previous = target;
    newElement->next = NULL;
    newElement->content = content;
}
void RemoveLastElement(List* list){
    List* target = list;
    while(target->next != NULL){
        target = target->next;
    }
    target->previous->next = NULL;
    free(target);
    
}
void RemoveElement(List* list,unsigned int index){
    List* target = list;
    //find the element to be removed
    for(unsigned int i = 0;i < index;i++){
        if(target->next == NULL){
            Error("List: Index out of range!\n");
            return;
        }
        target = target->next;
    }
    //remove the taget
    if(target->previous) target->previous->next = target->next;
    if(target->next) target->next->previous = target->previous;
    free(target);
    
}
void RemoveElementByContent(List* list, void* content){
    List* target = list;
    for(unsigned int i = 0;1;i++){
        if(target->content == content){
            RemoveElement(list,i);
            return;
        }
        if(target->next == NULL)
            break;
        target = target->next;
    }
    Error("List: Couldn't find element with content\n");
    return;
}