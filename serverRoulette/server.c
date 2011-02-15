/**
 * @file   server.c
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Main function per il server della Roulette
 *
 */

#include "../common/common_header.h"
#include "croupier.h"

int main(int argc, char **argv) {
    //TODO fare il join dei thread al termine
    //TODO controllare tutte le malloc!!!

    /* controllo numero di argomenti */
    if (argc != 3) {
        printf("Utilizzo: %s <numero porta> <intervallo secondi>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    int clientFd;
    client_t *datiConnessioneClient;
    short int serverPort; /* porta del server */
    int gameInterval; /* durata possibilità puntate */
    struct sockaddr_in self;
    struct sockaddr_in indirizzoClient;
    size_t lenIndirizzoClient;
    pthread_t playerTid, croupierTid;


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

        Pthread_create(&playerTid, NULL, player, (void *) datiConnessioneClient);
    }

    Close(sockfd);
    pthread_exit(NULL);
}