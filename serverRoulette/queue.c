/**
 * @file   queue.c
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include la definizione della struttura dati queue
 *
 */
#include <stdio.h>
#include "queue.h"

void queue_init(queue *myroot) {
    myroot->head = NULL;
    myroot->tail = NULL;
}

void queue_put(queue *myroot, node *mynode) {

    mynode->next = NULL;
    if (myroot->tail != NULL) {
        myroot->tail->next = mynode;
    }
    myroot->tail = mynode;

    if (myroot->head == NULL) {
        myroot->head = mynode;
    }
}

node *queue_get(queue* myroot) {
    //get from root
    node *mynode;
    mynode = myroot->head;
    if (myroot->head != NULL) {
        myroot->head = myroot->head->next;
    }
    return mynode;
}

void queue_remove(queue *myroot, node *mynode) {
    node *prev = myroot->head;
    if (prev != NULL) {
        //trovo il nodo precedente quello da rimuovere
        while ((prev->next != NULL) && (prev->next != mynode) ) {
            printf("prev: %p\nmynode: %p\n", prev, mynode);
            prev = prev->next;
        }
        prev->next = (prev->next)->next;
    }
}
