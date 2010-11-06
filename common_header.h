#ifndef _COMMON_HEADER_H
#define _COMMON_HEADER_H

#include <sys/types.h>       /* alcuni sistemi richiedono questo header file */
#include <sys/stat.h>
#include <stdio.h>     
#include <stdlib.h>

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


typedef struct player_tag {
	char				name[50];
	int					money;
	in_port_t			congrat_port;
} player_t;


typedef struct client_tag {
	struct sockaddr_in	client_data;	/* porta,indirizzo del client */
	int 				clientfd;		/* socket del client */
} client_t;

typedef struct counter_tag {
	int counter;
	pthread_mutex_t counter_mutex;
} counter_t;

/* ===PROVA LISTA RICHIESTE=== */
struct request {
    int number; //numero della richiesta
    struct request *next; //puntatore alla successiva richiesta
};

void add_request(int request_num, pthread_mutex_t *mutex, pthread_cond_t *cond);
struct request *get_request(pthread_mutex_t *mutex);
void handle_request(struct request* a_request, pthread_t thread_id);


/* ===FINE PROVA LISTA RICHIESTE=== */

/**
 * Stampa sullo standard error un messaggio contenente l'errore, il file che l'ha
 * causato, la riga e la spiegazione.
 * 
 * @param code il codice d'errore (il valore di ritorno per le funzioni pthread,
 *		la variabile errno per le altre syscall)
 * @param text Una stringa che spiega l'errore
 */
void err_abort(int code, char *text);




#endif  /* _COMMON_HEADER_H */

