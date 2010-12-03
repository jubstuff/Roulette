/* 
 * File:   queue.h
 * Author: just
 *
 * Created on 9 novembre 2010, 23.29
 */

#ifndef QUEUE_H
#define	QUEUE_H

typedef struct node_tag {
    struct node *next;
} node;

typedef struct queue {
    node *head, *tail;
} queue;

void queue_init(queue *myroot);
void queue_put(queue *myroot, node *mynode);
node *queue_get(queue *myroot);


#endif	/* QUEUE_H */

