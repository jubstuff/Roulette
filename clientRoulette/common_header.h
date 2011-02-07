#ifndef _COMMON_HEADER_H
#define _COMMON_HEADER_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h> //ETIMEDOUT
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

#define MAXBUF 4096 /* max line length */
#define MAX_BUDGET 500
#define NICK_LENGTH 50
#define IP_ADDRESS_LENGTH 15
/*
 * Definizione dei tipi
 */

/**
 * client_t
 *
 * Contiene i dettagli di connessione per un singolo client connesso al server
 */
typedef struct client_tag {
    struct sockaddr_in clientData; // porta,indirizzo del client
    int clientFd; // socket del client
} client_t;

/**
 * sessione_puntate_t
 *
 * Contiene le informazioni sullo stato delle puntate della sessione corrente.
 * In ogni momento dell'esecuzione ne esiste una ed una sola istanza chiamata
 * sessionePuntateCorrente
 */
typedef struct sessioneDiPuntate {
    pthread_mutex_t mutex;
    pthread_cond_t aperte;
    pthread_cond_t chiuse;
    pthread_cond_t attesaCroupier;
    int stato;
} sessione_puntate_t;
/**
 * sessione_gioco_t
 *
 * Contiene le informazioni su una sessione di gioco della Roulette
 * In ogni momento dell'esecuzione ne esiste una ed una sola istanza chiamata
 * sessioneGiocoCorrente
 */
typedef struct sessioneDiGioco {
    queue elencoGiocatori;
    pthread_mutex_t mutex;
    pthread_cond_t attesaRiempimentoListaPuntate;
    pthread_cond_t attesaAlmenoUnGiocatore;
    int giocatoriConnessi;
    int giocatoriChePuntano;
} sessione_gioco_t;

/**
 * player_t
 *
 * Contiene i dettagli su un giocatore connesso al server
 */
typedef struct player {
    struct node *next;
    int budgetPrecedente;
    int budgetAttuale;
    char nickname[NICK_LENGTH];
    in_port_t portaMessaggiCongratulazioni;
    client_t *datiConnessioneClient;
    queue elencoPuntate;
} player_t;

/**
 * puntata_t
 *
 * Elemento della lista puntate. Contiene i dettagli della puntata.
 * Il tipo puntata può assumere valori
 * -1 => Dispari
 * -2 => Pari
 * >=0 => Numero
 */
typedef struct puntate_node {
    struct node *next;
    int numeroPuntato;
    int tipoPuntata;
    int sommaPuntata;
} puntata_t;

/**
 * vincitore_t
 * 
 * Elemento della lista vincitori. 
 */
typedef struct vincitore {
    int portaMessaggiCongratulazioni;
    struct sockaddr_in indirizzoIp;
}vincitore_t;

/**
 * analisi_puntata_t
 *
 * Contiene l'analisi per una giocata fatta da tutti i giocatori connessi.
 * Viene riazzerata ad ogni puntata.
 */
typedef struct analisiDiSessionePuntata{
    queue elencoVincitori;
    int numeroPerdenti;
    int numeroVincitori;
    pthread_mutex_t mutex;
    pthread_cond_t attesaMessaggi;
} analisi_puntata_t;

/*
 * Definizione variabili
 */

extern int num_requests;
extern int stato_puntate;
extern int numero_di_vincitori_in_questa_mano;
extern int numero_di_perdenti_in_questa_mano;

sessione_gioco_t sessioneGiocoCorrente;
sessione_puntate_t sessionePuntateCorrente;
analisi_puntata_t analisiSessionePuntata;

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


//void gestisci_puntate(int estratto);
void gestisci_puntata_numero(int estratto, puntata_t *puntata, player_t *player);
void gestisci_puntata_pari(int estratto, puntata_t *puntata, player_t *player);
void gestisci_puntata_dispari(int estratto, puntata_t *puntata, player_t *player);
void aumenta_budget(int moltiplicatore, puntata_t *puntata, player_t *player);

int roulette_mutex_lock(pthread_mutex_t *mutex, char *msg);
int roulette_mutex_unlock(pthread_mutex_t *mutex, char *msg);

//puntata_t *inizializza_nodo_puntata(int numero_puntato, bet_t tipo_puntata, int somma_puntata);


struct timespec calcola_intervallo(int intervallo);
#endif /* _COMMON_HEADER_H */
