/**
 * @file   common_header.h
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include tutte le definizioni comuni al progetto
 *
 */


#ifndef _COMMON_HEADER_H
#define _COMMON_HEADER_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h> //ETIMEDOUT
#include <fcntl.h>
#include "../serverRoulette/player.h"
#include <pthread.h>
#include "../serverRoulette/queue.h"
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

/**
 * Massima lunghezza di un buffer di testo
 */
#define MAXBUF 4096
/**
 * Massimo budget ammesso dal server //TODO integrarlo nella giocata
 */
#define MAX_BUDGET 500
/**
 * Lunghezza massima di un nickname per un giocatore
 */
#define NICK_LENGTH 100
/**
 * Lunghezza di un indirizzo IP in formato ASCII
 */
#define IP_ADDRESS_LENGTH 15

/*
 * Definizione dei tipi
 */

/**
 * client_t
 *
 * Contiene i dettagli di connessione per un singolo client connesso al server
 *
 * @param clientData porta e indirizzo del client
 * @param clientFd socket di comunicazione tra client e thread player
 */
typedef struct client_tag {
    struct sockaddr_in clientData;
    int clientFd;
} client_t;

/**
 * sessione_puntate_t
 *
 * Contiene le informazioni sullo stato delle puntate della sessione corrente.
 * In ogni momento dell'esecuzione ne esiste una ed una sola istanza chiamata
 * sessionePuntateCorrente
 *
 * @param mutex Mutex associato alla sessione delle puntate
 * @param aperte Condition variable che indica l'attesa per l'apertura delle puntate
 * @param chiuse Condition variable che indica l'attesa per la chiusura delle puntate
 * @param attesaCroupier Condition variable utilizzata per simulare lo scadere del tempo 
 * @param stato Informa sullo stato delle puntate. 1 significa puntate aperte,
 * 0 significa puntate chiuse
 *
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
 *
 * @param elencoGiocatori Lista contenente tutti i giocatori connessi
 * @param mutex Mutex associato alla sessione di gioco
 * @param attesaRiempimentoListaPuntate Condition variable che indica l'attesa
 * del croupier per il riempimento della lista puntate da parte dei player
 * @param attesaAlmenoUnGiocatore Condition variable che indica l'attesa da
 * parte del croupier per la connessione di un numero minimo di giocatori
 * @param giocatoriConnessi Numero di giocatori connessi al server
 * @param giocatoriChePuntano Numero di giocatori che partecipano alla puntata
 * corrente
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
 * Nodo della lista sessioneDiGioco#elencoGiocatori.
 * Contiene i dettagli su un giocatore connesso al server
 * @param next Puntatore al successivo nodo nella lista
 * @param budgetPrecedente Budget del giocatore prima dell'analisi della sessione
 * di puntate da parte del croupier
 * @param budgetAttuale Budget reale del giocatore, modificato durante l'analisi
 * della sessione di puntate da parte del croupier
 * @param nickname Nickname del giocatore
 * @param portaMessaggiCongratulazioni Porta sulla quale il client del giocatore
 * è posto in ascolto per accettare i messaggi di congratulazione
 * @param datiConnessioneClient Contiene i dati di connessione del client, porta
 * e indirizzo IP
 * @param elencoPuntate Lista contenente tutte le puntate del giocatore per la
 * sessione corrente
 * @param vincitore Indica se il giocatore ha vinto la puntata corrente;
 * 1 se ha vinto, 0 se ha perso
 */
typedef struct player {
    struct node *next;
    int budgetPrecedente;
    int budgetAttuale;
    char nickname[NICK_LENGTH];
    in_port_t portaMessaggiCongratulazioni;
    client_t *datiConnessioneClient;
    queue elencoPuntate;
    int vincitore;
} player_t;

/**
 * puntata_t
 *
 * Nodo della lista puntate. Contiene i dettagli della puntata.
 * @param next Puntatore al successivo nodo nella lista
 * @param numeroPuntato Se la puntata è di tipo numerico, contiene il numero 
 * puntato dal giocatore
 * @param tipoPuntata Contiene il tipo di puntata. Può assumere i valori:
 * -1 -> Dispari;
 * -2 -> Pari;
 * >=0 <=36 -> Numero
 * @param sommaPuntata Somma puntata dal giocatore
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
 * @param next Puntatore al successivo nodo nella lista
 * @param portaMessaggiCongratulazioni Contiene la porta su cui il client
 * associato è in ascolto per i messaggi di congratulazioni
 * @param indirizzoIp Contiene l'indirizzo del client associato
 */
typedef struct vincitore {
    struct node *next;
    int portaMessaggiCongratulazioni;
    struct sockaddr_in indirizzoIp;
} vincitore_t;

/**
 * analisi_puntata_t
 *
 * Contiene l'analisi per una giocata fatta da tutti i giocatori connessi.
 * Viene riazzerata ad ogni puntata.
 *
 * @param elencoVincitori Lista contenente tutti i vincitori della sessione di 
 * puntate corrente
 * @param numeroPerdenti Numero di perdenti nella sessione di puntate corrente
 * @param numeroVincitori Numero di vincitori nella sessione di puntate corrente
 * @param stato Indica se il croupier ha analizzato la sessione di puntate.
 * 0 se deve ancora analizzare la sessione, 1 se ha terminato l'analisi.
 * @param mutex Mutex associato alla sessione di analisi
 * @param attesaMessaggi Condition variable che indica l'attesa di un player per
 * poter intraprendere la fase di scambio messaggi di congratulazioni
 */
typedef struct analisiDiSessionePuntata {
    queue elencoVincitori;
    int numeroPerdenti;
    int numeroVincitori;
    int stato;
    pthread_mutex_t mutex;
    pthread_cond_t attesaMessaggi;
} analisi_puntata_t;

/*
 * Definizione variabili
 */
//TODO eliminare queste variabili se sono inutili
//extern int num_requests;
//extern int stato_puntate;
//extern int numero_di_vincitori_in_questa_mano;
//extern int numero_di_perdenti_in_questa_mano;

sessione_gioco_t sessioneGiocoCorrente;
sessione_puntate_t sessionePuntateCorrente;
analisi_puntata_t analisiSessionePuntata;

extern const char messaggioPuntateAperte[];
extern ssize_t lenMessaggioPuntateAperte;
extern const char messaggioPuntateChiuse[];
extern ssize_t lenMessaggioPuntateChiuse;

extern int numeroMinimoGiocatori;

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

//puntata_t *inizializza_nodo_puntata(int numero_puntato, bet_t tipo_puntata, int somma_puntata);


struct timespec calcola_intervallo(int intervallo);


/******************************************************************************
 * =WRAPPER FUNCTIONS=
 */
//socket
int Socket(int domain, int type, int protocol);
//bind
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
//listen
void Listen(int sockfd, int backlog);
//accept
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
//connect
void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
//close
void Close(int fildes);
//malloc
void *Malloc(size_t size);
//pthread_mutex_init
void Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
//pthread_cond_init
void Pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
//pthread_create
void Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
        void *(*start_routine)(void*), void *arg);
//pthread_cancel
void Pthread_cancel(pthread_t thread);
//pthread_mutex_lock
void Pthread_mutex_lock(pthread_mutex_t *mutex);
//pthread_mutex_unlock
void Pthread_mutex_unlock(pthread_mutex_t *mutex);
//pthread_cond_wait
void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
//pthread_cond_timedwait
int Pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
        const struct timespec *abstime);
//pthread_cond_broadcast
void Pthread_cond_broadcast(pthread_cond_t *cond);
//pthread_cond_signal
void Pthread_cond_signal(pthread_cond_t *cond);
//write
ssize_t Write(int fd, const void *buf, size_t count);
//read
ssize_t Read(int fd, void *buf, size_t count);
//TODO getsockname
//TODO inet_aton
//TODO htons
//TODO pipe
//TODO fork
//TODO wait


#endif /* _COMMON_HEADER_H */
