/* estrazione.c
 *
 * TODO inserire descrizione file
 */
#define DEBUG 1


#include "../common/common_header.h"
#include "croupier.h"


//TODO inserire descrizione funzione

int main(int argc, char **argv) {
    //TODO fare il join dei thread al termine
    //TODO controllare tutte le malloc!!!

    /* controllo numero di argomenti */
    if (argc != 3) {
        printf("Utilizzo: %s <numero porta> <intervallo secondi>\n", argv[0]);
        exit(1);
    } //TODO controllo errori più robusto

    int sockfd;
    int clientFd;
    client_t *datiConnessioneClient;
    short int serverPort; /* porta del server */
    int gameInterval; /* durata possibilità puntate */
    struct sockaddr_in self;
    struct sockaddr_in indirizzoClient;
    size_t lenIndirizzoClient;
    int status; /* raccoglie i valori restituiti dalle system call */
    pthread_t playerTid, croupierTid;

    //TODO mettere queste inizializzazioni in una funzione
    //TODO controllo errori sulle inizializzazioni
    /*
     * INIZIALIZZAZIONI
     */
    //inizializzo la sessione di gioco corrente
    Pthread_mutex_init(&sessioneGiocoCorrente.mutex, NULL);
    Pthread_cond_init(&sessioneGiocoCorrente.attesaAlmenoUnGiocatore, NULL);
    Pthread_cond_init(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate, NULL);
    queue_init(&sessioneGiocoCorrente.elencoGiocatori);
    sessioneGiocoCorrente.giocatoriChePuntano = 0;
    sessioneGiocoCorrente.giocatoriConnessi = 0;

    //inizializzo la sessione di puntate
    Pthread_mutex_init(&sessionePuntateCorrente.mutex, NULL);
    Pthread_cond_init(&sessionePuntateCorrente.aperte, NULL);
    Pthread_cond_init(&sessionePuntateCorrente.chiuse, NULL);
    Pthread_cond_init(&sessionePuntateCorrente.attesaCroupier, NULL);
    sessionePuntateCorrente.stato = 0;

    //inizializzo l'analisi della sessione
    Pthread_mutex_init(&analisiSessionePuntata.mutex, NULL);
    Pthread_cond_init(&analisiSessionePuntata.attesaMessaggi, NULL);
    queue_init(&analisiSessionePuntata.elencoVincitori);
    analisiSessionePuntata.numeroPerdenti = 0;
    analisiSessionePuntata.numeroVincitori = 0;
    analisiSessionePuntata.stato = 0;
    //inizializzo la generazione random
    srand(time(NULL));

    /* converto i parametri passati a interi */
    serverPort = atoi(argv[1]);
    gameInterval = atoi(argv[2]);

    //FINE INIZIALIZZAZIONI

    printf("La giocata durerà %d secondi\n", gameInterval);

    /* creo il thread CROUPIER */
    Pthread_create(&croupierTid, NULL, croupier, (void *) gameInterval);
    
    sockfd = open_socket(self, serverPort);

    while (1) {
        /* accetta connessioni dai client */
        lenIndirizzoClient = sizeof (indirizzoClient);
        clientFd = Accept(sockfd, (struct sockaddr *) &indirizzoClient, &lenIndirizzoClient);
        
        /* inserisco informazioni sul client da inviare al thread player */
        datiConnessioneClient = NULL;
        datiConnessioneClient = Malloc(sizeof (client_t)); //TODO Quando fare la free di questo?
        datiConnessioneClient->clientData = indirizzoClient;
        datiConnessioneClient->clientFd = clientFd;

        status = pthread_create(&playerTid, NULL, player, (void *) datiConnessioneClient);
        if (status != 0) {
            err_abort(status, "Creazione thread");
        }
    }

    close(sockfd);
    pthread_exit(NULL);
}

#ifndef DEBUG
/* Crea 10 player thread */
for (j = 0; j < 10; j++) {
    status = pthread_create(&playerTid, NULL, player, (void *) j);
    if (status != 0) {
        err_abort(status, "Creazione thread");
    }
}
#endif