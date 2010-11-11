#ifndef _COMMON_HEADER_H
#define _COMMON_HEADER_H

#include <sys/types.h>       /* alcuni sistemi richiedono questo header file */
#include <sys/stat.h>
#include <stdio.h>     
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>    /* for convenience */
#include <unistd.h>    /* for convenience */
#include <signal.h>    /* for SIG_ERR */
#include <time.h>
#define MAXBUF 4096               /* max line length */


#include <wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* thread */
#include <pthread.h>

#include <errno.h>

extern int num_requests;
extern int estratto;


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

