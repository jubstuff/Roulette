#include <stdio.h>
#include "queue.h"

void queue_init(queue *myroot) {
	myroot->head = NULL;
	myroot->tail = NULL;
}

void queue_put(queue *myroot, node *mynode) {
	mynode->next = NULL;
	if(myroot->tail != NULL) {
		myroot->tail->next = mynode;
	}
	myroot->tail = mynode;

	if(myroot->head == NULL) {
		myroot->head = mynode;
	}
}

node *queue_get(queue* myroot) {
	//get from root
	node *mynode;
	mynode = myroot->head;
	if(myroot->head != NULL) {
		myroot->head = myroot->head->next;
	}
	return mynode;
}
