#ifndef _COMMON_HEADER_H
#define _COMMON_HEADER_H

#include <arpa/inet.h>
#include <ctype.h>
#include "control.h"
#include <errno.h> //ETIMEDOUT
#include <fcntl.h>
#include "player.h"
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

#define MAXBUF 4096 /* max line length */
#define MAX_BUDGET 500

/**
 * Definizione dei tipi
 */
typedef enum bet_type {
    NUMBER = 0,
    EVEN,
    ODD
} bet_t;

typedef enum bet_state {
    OPEN = 1,
    CLOSED
} state_t;

typedef struct client_tag {
    struct sockaddr_in client_data; // porta,indirizzo del client
    int clientfd; // socket del client
} client_t;

typedef struct lista_puntate {
    data_control control;
    queue puntate;
    int stato_puntate;
} lista_puntate_t;

typedef struct lista_giocatori {
    data_control control;
    queue giocatori;
    int num_giocatori;
} lista_giocatori_t;

typedef struct player_tag {
    struct node *next;
    int money;
    char nickname[50]; //FIXME inserire una costante al posto di 50
    int win_money; //JUST_ASK questo cos'è?
    int messport;
    client_t *info_client;
    lista_puntate_t lista_puntate_personale;
} player_t;

typedef struct puntate_node {
    struct node *next;
    int numero;
    bet_t tipo;
    int somma_puntata;
} puntata_t;

/**
 * Definizione variabili
 */

extern int num_requests;

extern int stato_puntate;

extern int numero_di_vincitori_in_questa_mano;

extern int numero_di_perdenti_in_questa_mano;


lista_giocatori_t players_list;
lista_puntate_t lista_puntate;


/**
 * Stampa sullo standard error un messaggio contenente l'errore, il file che l'ha
 * causato, la riga e la spiegazione.
 *
 * @param code il codice d'errore (il valore di ritorno per le funzioni pthread,
 * la variabile errno per le altre syscall)
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


void gestisci_puntate(int estratto);
void gestisci_puntata_numero(int estratto, puntata_t *puntata, player_t *player);
void gestisci_puntata_pari(int estratto, puntata_t *puntata, player_t *player);
void gestisci_puntata_dispari(int estratto, puntata_t *puntata, player_t *player);
void aumenta_budget(int moltiplicatore, puntata_t *puntata, player_t *player);

puntata_t *inizializza_nodo_puntata(int numero_puntato, bet_t tipo_puntata, int somma_puntata);


struct timespec calcola_intervallo(int intervallo);
#endif /* _COMMON_HEADER_H */