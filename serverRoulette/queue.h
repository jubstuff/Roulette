/**
 * @file   queue.h
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include la definizione della struttura dati queue
 *
 */

#ifndef QUEUE_H
#define	QUEUE_H
/**
 * node_tag
 *
 * Un nodo di una lista. Rappresenta il caso base, ovvero un nodo contenente
 * solo il puntatore al nodo successivo. Verrà utilizzato come "superclasse"
 * per tutti gli altri tipi di nodo
 *
 * @param next Puntatore al nodo successivo
 */
typedef struct node_tag {
    struct node_tag *next;
} node;

/**
 * queue
 *
 * Struttura dati contenente due nodi. Uno rappresenterà la testa, l'altro la
 * coda della lista.
 *
 * @param head Testa della coda
 * @param tail Coda
 */
typedef struct queue {
    node *head;
    node *tail;
} queue;

/**
 * queue_init
 *
 * Inizializza testa e coda a NULL
 *
 * @param myroot queue da inizializzare
 */
void queue_init(queue *myroot);

/**
 * queue_put
 *
 * Inserisce un nodo nella coda
 *
 * @param myroot Coda a cui aggiungere il nodo
 * @param mynode Nodo da aggiungere
 */
void queue_put(queue *myroot, node *mynode);

/**
 * queue_get
 *
 * Rimuove un nodo dalla coda
 * @param myroot Coda da cui rimuovere il nodo
 * @return Puntatore al nodo rimosso
 */
node *queue_get(queue *myroot);

void queue_remove(queue *myroot, node *mynode);

#endif	/* QUEUE_H */

