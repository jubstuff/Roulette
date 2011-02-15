/**
 * @file   common_header.h
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include tutte le definizioni comuni al progetto
 *
 */

/**
 * @mainpage Roulette
 *
 * @authors Giustino Borzacchiello, Antonio Cifariello, Francesco Paolo Cimmino
 *
 * @section intro Introduzione
 *
 * Questa è la pagina iniziale del progetto Roulette
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
 *  - -1 -> Dispari;
 *  - -2 -> Pari;
 *  - >=0 <=36 -> Numero
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

/**
 * sessioneGiocoCorrente
 *
 * Gestisce la sessione di gioco corrente
 */
sessione_gioco_t sessioneGiocoCorrente;

/**
 * sessionePuntateCorrente
 * 
 * Gestisce la sessione di puntate corrente
 */
sessione_puntate_t sessionePuntateCorrente;

/**
 * analisiSessionePuntata
 *
 * Gestisce l'analisi della sessione di puntate corrente
 */
analisi_puntata_t analisiSessionePuntata;

/**
 * messaggioPuntateAperte
 *
 * Messaggio che il thread player invia al client per indicare l'apertura delle
 * puntate
 */
extern const char messaggioPuntateAperte[];

/**
 * lenMessaggioPuntateAperte
 *
 * Dimensione del messaggio di puntate aperte
 */
extern ssize_t lenMessaggioPuntateAperte;

/**
 * messaggioPuntateChiuse
 *
 * Messaggio che il thread player invia al client per indicare la chiusura delle
 * puntate
 */
extern const char messaggioPuntateChiuse[];

/**
 * lenMessaggioPuntateChiuse
 *
 * Dimensione del messaggio di puntate chiuse
 */
extern ssize_t lenMessaggioPuntateChiuse;

/**
 * numeroMinimoGiocatori
 *
 * Numero minimo di giocatori connessi che il server attende per iniziare il
 * gioco
 */
extern int numeroMinimoGiocatori;

/**
 * err_abort
 * 
 * Stampa sullo standard error un messaggio contenente l'errore, il file che l'ha
 * causato, la riga e la spiegazione.
 *
 * @param code il codice d'errore (il valore di ritorno per le funzioni pthread,
 * la variabile errno per le altre syscall)
 * @param text Una stringa che spiega l'errore
 */
void err_abort(int code, char *text);

/**
 * open_socket
 * 
 * Apre un socket descriptor, gli assegna un nome e si mette in ascolto.
 *
 * @param self struttura che conterrà le informazioni relative al server
 * @param server_port porta sulla quale mettere in ascolto il server
 */
int open_socket(struct sockaddr_in self, short int server_port);

/**
 * gestisci_puntata_numero
 *
 * Gestisce una puntata di tipo numerico, aumentando il budget del giocatore
 * nel caso in cui sia vincente.
 * 
 * @param estratto Numero estratto nella sessione di gioco corrente
 * @param puntata Puntata effettuata dal giocatore
 * @param player Giocatore che ha effettuato la puntata
 */
void gestisci_puntata_numero(int estratto, puntata_t *puntata, player_t *player);

/**
 * gestisci_puntata_pari
 *
 * Gestisce una puntata di tipo pari, aumentando il budget del giocatore
 * nel caso in cui sia vincente.
 *
 * @param estratto Numero estratto nella sessione di gioco corrente
 * @param puntata Puntata effettuata dal giocatore
 * @param player Giocatore che ha effettuato la puntata
 */
void gestisci_puntata_pari(int estratto, puntata_t *puntata, player_t *player);

/**
 * gestisci_puntata_dispari
 *
 * Gestisce una puntata di tipo dispari, aumentando il budget del giocatore
 * nel caso in cui sia vincente.
 *
 * @param estratto Numero estratto nella sessione di gioco corrente
 * @param puntata Puntata effettuata dal giocatore
 * @param player Giocatore che ha effettuato la puntata
 */
void gestisci_puntata_dispari(int estratto, puntata_t *puntata, player_t *player);

/**
 * aumenta_budget
 * 
 * Aumenta il budget del giocatore di un dato moltiplicatore
 * 
 * @param moltiplicatore Moltiplicatore del budget
 * @param puntata Puntata //TODO possibile passare un int invece di tutta la puntata
 * @param player //TODO possibile passare il budget invece del giocatore
 */
void aumenta_budget(int moltiplicatore, puntata_t *puntata, player_t *player);

/**
 * Calcola l'intervallo di attesa del croupier
 * @param intervallo
 * @return la struttura timespec contenente il tempo di fine attesa
 */
struct timespec calcola_intervallo(int intervallo);

/*
 * =WRAPPER FUNCTIONS=
 */
/**
 * Socket
 *
 * Wrapper function per la system call socket. Termina il programma se si
 * verifica un errore
 *
 * @param domain
 * @param type
 * @param protocol
 * @return
 */
int Socket(int domain, int type, int protocol);

/**
 * Bind
 *
 * Wrapper function per la system call bind. Termina il programma se si
 * verifica un errore
 *
 * @param sockfd
 * @param addr
 * @param addrlen
 */
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * Listen
 *
 * Wrapper function per la system call listen. Termina il programma se si
 * verifica un errore
 *
 * @param sockfd
 * @param addr
 * @param addrlen
 */
void Listen(int sockfd, int backlog);

/**
 * Accept
 *
 * Wrapper function per la system call accept. Termina il programma se si
 * verifica un errore
 *
 * @param sockfd
 * @param addr
 * @param addrlen
 */
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/**
 * Connect
 *
 * Wrapper function per la system call connect. Termina il programma se si
 * verifica un errore
 * 
 * @param sockfd
 * @param addr
 * @param addrlen
 */
void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * Close
 *
 * Wrapper function per la system call close. Termina il programma se si
 * verifica un errore
 *
 * @param fildes
 */
void Close(int fildes);

/**
 * Malloc
 *
 * Wrapper function per la system call malloc. Termina il programma se si
 * verifica un errore
 *
 * @param mutex
 * @param attr
 */
void *Malloc(size_t size);

/**
 * Pthread_mutex_init
 * 
 * Wrapper function per la system call pthread_mutex_init. Termina il programma
 * se si verifica un errore
 *
 * @param thread
 * @param attr
 * @param start_routine
 * @param arg
 */
void Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

/**
 * Pthread_cond_init
 *
 * Wrapper function per la system call pthread_cond_init. Termina il programma
 * se si verifica un errore
 *
 * @param cond
 * @param attr
 */
void Pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

/**
 * Pthread_create
 *
 * Wrapper function per la system call pthread_create. Termina il programma
 * se si verifica un errore
 *
 * @param thread
 */
void Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
        void *(*start_routine)(void*), void *arg);

/**
 * Pthread_cancel
 *
 * Wrapper function per la system call pthread_cancel. Termina il programma
 * se si verifica un errore
 *
 * @param thread
 */
void Pthread_cancel(pthread_t thread);

/**
 * Pthread_mutex_lock
 *
 * Wrapper function per la system call pthread_mutex_lock. Termina il programma
 * se si verifica un errore
 *
 * @param mutex
 */
void Pthread_mutex_lock(pthread_mutex_t *mutex);

/**
 * Pthread_mutex_unlock
 *
 * Wrapper function per la system call pthread_mutex_unlock. Termina il programma
 * se si verifica un errore
 *
 * @param mutex
 */
void Pthread_mutex_unlock(pthread_mutex_t *mutex);

/**
 * Pthread_cond_wait
 *
 * Wrapper function per la system call pthread_cond_wait. Termina il programma
 * se si verifica un errore
 *
 * @param cond
 * @param mutex
 */
void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

/**
 * Pthread_cond_timedwait
 *
 * Wrapper function per la system call pthread_cond_timedwait. Termina il programma
 * se si verifica un errore
 *
 * @param cond
 */
int Pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
        const struct timespec *abstime);

/**
 * Phtread_cond_broadcast
 *
 * Wrapper function per la system call pthread_cond_broadcast. Termina il programma
 * se si verifica un errore
 *
 * @param cond
 */
void Pthread_cond_broadcast(pthread_cond_t *cond);

/**
 * Pthread_cond_signal
 *
 * Wrapper function per la system call pthread_cond_signal. Termina il programma
 * se si verifica un errore
 *
 * @param cond
 */
void Pthread_cond_signal(pthread_cond_t *cond);

/**
 * Write
 *
 * Wrapper function per la system call write. Termina il programma
 * se si verifica un errore
 *
 * @param fd
 * @param buf
 * @param count
 * @return
 */
ssize_t Write(int fd, const void *buf, size_t count);

/**
 * Read
 *
 * Wrapper function per la system call read. Termina il programma
 * se si verifica un errore
 *
 * @param fd
 * @param buf
 * @param count
 * @return
 *
 */
ssize_t Read(int fd, void *buf, size_t count);

/**
 * Getsockname
 *
 * Wrapper function per la system call Getsockname. Termina il programma se si
 * verifica un errore
 * 
 * @param sockfd
 * @param addr
 * @param addrlen
 */
void Getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/**
 * Pipe
 *
 * Wrapper function per la system call Pipe. Termina il programma se si verifica
 * un errore
 * 
 * @param fildes
 */
void Pipe(int fildes[2]);

//TODO wait


#endif /* _COMMON_HEADER_H */
