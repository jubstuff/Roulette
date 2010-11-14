#ifndef _COMMON_HEADER_H
#define _COMMON_HEADER_H

#include <arpa/inet.h>
#include <ctype.h>
#include "control.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "queue.h"
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>     
#include <stdlib.h>
#include <string.h>    
#include <time.h>
#include <unistd.h>    
#include <wait.h>

#define MAXBUF 4096               /* max line length */



/* thread */

extern int num_requests;
extern int estratto;


struct lista_puntate {
	data_control control;
	queue puntate;
} bl;

typedef struct puntate_node {
	struct node *next;
	int puntata;
} pnode;


/**
 * Stampa sullo standard error un messaggio contenente l'errore, il file che l'ha
 * causato, la riga e la spiegazione.
 * 
 * @param code il codice d'errore (il valore di ritorno per le funzioni pthread,
 *		la variabile errno per le altre syscall)
 * @param text Una stringa che spiega l'errore
 */
void err_abort(int code, char *text);

/**
 * Apre un socket descriptor, gli assegna un nome e si mette in ascolto.
 *
 * @param self struttura che conterrà le informazioni relative al server
 * @param server_port porta sulla quale mettere in ascolto il server
 */
int open_socket(struct sockaddr_in self, short int server_port);

#endif  /* _COMMON_HEADER_H */

