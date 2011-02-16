/**
 * @file   player.c
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Procedure per il thread player e il gestore delle puntate
 *
 */
#include "../common/common_header.h"
#include "player.h"

/**
 * FUNZIONE player
 *
 *============================================================================*/
void *player(void *arg) {
    player_t *datiGiocatore;
    pthread_t tidGestorePuntateGiocatore;
    queue listaPuntatePrivata;
    argomento_gestore_puntate_t *argomentoGestorePuntate;
    size_t nicknameLen;
    int numeroVincitori;
    /*
     * Quando è già in atto una puntata, non si possono connettere nuovi giocatori
     * Aspetta che le puntate siano chiuse per connettersi
     */
    Pthread_mutex_lock(&sessionePuntateCorrente.mutex);
    while (sessionePuntateCorrente.stato == 1) {
        Pthread_cond_wait(&sessionePuntateCorrente.chiuse,
            &sessionePuntateCorrente.mutex);
    }
    Pthread_mutex_unlock(&sessionePuntateCorrente.mutex);


    /*
     * Recupera dati dal client e inserisci un nuovo giocatore nella lista
     */
    datiGiocatore = (player_t *) Malloc(sizeof (player_t));

    datiGiocatore->datiConnessioneClient = (client_t *) arg;
    Read(datiGiocatore->datiConnessioneClient->clientFd,
            &datiGiocatore->portaMessaggiCongratulazioni, sizeof (in_port_t));
    Read(datiGiocatore->datiConnessioneClient->clientFd,
            &datiGiocatore->budgetAttuale, sizeof (int));
    Read(datiGiocatore->datiConnessioneClient->clientFd,
            &nicknameLen, sizeof (size_t));
    Read(datiGiocatore->datiConnessioneClient->clientFd,
            datiGiocatore->nickname, nicknameLen);

    datiGiocatore->portaMessaggiCongratulazioni =
            ntohs(datiGiocatore->portaMessaggiCongratulazioni);
    datiGiocatore->budgetPrecedente = 0;
    datiGiocatore->vincitore = 0;

    printf("====Dati Giocatore====\n");
    printf("Nickname: %s\n", datiGiocatore->nickname);
    printf("Budget Iniziale: %d\n", datiGiocatore->budgetAttuale);
    printf("Porta Congratulazioni: %d\n\n",
            htons(datiGiocatore->portaMessaggiCongratulazioni));

    Pthread_mutex_lock(&sessioneGiocoCorrente.mutex);
    queue_put(&sessioneGiocoCorrente.elencoGiocatori, (node *) datiGiocatore);
    sessioneGiocoCorrente.giocatoriConnessi++;
    /*
     * Se questo thread è il primo giocatore connesso, avvisa il croupier che
     * può iniziare la giocata
     */
    if (sessioneGiocoCorrente.giocatoriConnessi == numeroMinimoGiocatori) {
        Pthread_cond_signal(&sessioneGiocoCorrente.attesaAlmenoUnGiocatore);
    }
    Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);

    /*
     * Prepara l'argomento da passare al thread gestore delle puntate
     */
    argomentoGestorePuntate =
        (argomento_gestore_puntate_t *) Malloc(sizeof (argomento_gestore_puntate_t));
    queue_init(&listaPuntatePrivata);
    argomentoGestorePuntate->listaPuntatePrivata = &listaPuntatePrivata;
    argomentoGestorePuntate->clientFd = datiGiocatore->datiConnessioneClient->clientFd;

    while (1) {
        /*
         * Aspetta che il croupier apra le puntate
         */
        printf("[Player] Aspetto l'apertura delle puntate\n");
        Pthread_mutex_lock(&sessionePuntateCorrente.mutex);
        while (sessionePuntateCorrente.stato == 0) {
            Pthread_cond_wait(&sessionePuntateCorrente.aperte,
                    &sessionePuntateCorrente.mutex);
        }
        Pthread_mutex_unlock(&sessionePuntateCorrente.mutex);

        printf("[Player] Creo il gestore delle puntate\n");
        Pthread_create(&tidGestorePuntateGiocatore, NULL,
                gestorePuntateGiocatore, (void *) argomentoGestorePuntate);

        /*
         * Aspetta che il croupier chiuda le puntate
         */
        Pthread_mutex_lock(&sessionePuntateCorrente.mutex);
        while (sessionePuntateCorrente.stato == 1) {
            Pthread_cond_wait(&sessionePuntateCorrente.chiuse,
                    &sessionePuntateCorrente.mutex);
        }
        Pthread_mutex_unlock(&sessionePuntateCorrente.mutex);

        Pthread_cancel(tidGestorePuntateGiocatore);


        /*
         * collegare pacchetto di puntate alla lista del giocatore
         */
        Pthread_mutex_lock(&sessioneGiocoCorrente.mutex);
        /* aggiunge la lista recuperata dal gestore puntate al nodo relativo al
         * giocatore nella lista globale */
        datiGiocatore->elencoPuntate.head = listaPuntatePrivata.head;
        sessioneGiocoCorrente.giocatoriChePuntano--;
        /*
         * Se tutti i giocatori hanno collegato il proprio pacchetto di puntate
         * risveglia il croupier
         */
        if (sessioneGiocoCorrente.giocatoriChePuntano == 0) {
            Pthread_cond_signal(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate);
        }
        Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);

        queue_init(&listaPuntatePrivata);
        queue_init(argomentoGestorePuntate->listaPuntatePrivata);

        //* Aspettare che il croupier gestisca le puntate
        //* Quando segnalato, far partire la gestione dei messaggi tra client

        Pthread_mutex_lock(&analisiSessionePuntata.mutex);
        while (analisiSessionePuntata.stato == 0) {
            printf("[player] Aspetto che termini la gestione delle puntaten\n");
            Pthread_cond_wait(&analisiSessionePuntata.attesaMessaggi,
                    &analisiSessionePuntata.mutex);
        }
        Pthread_mutex_unlock(&analisiSessionePuntata.mutex);

        //=gestione messaggi tra client

        Pthread_mutex_lock(&sessioneGiocoCorrente.mutex);

        if (datiGiocatore->vincitore == 1) {
            //vinto
            Write(datiGiocatore->datiConnessioneClient->clientFd,
                    &(datiGiocatore->vincitore), sizeof (int));
            Write(datiGiocatore->datiConnessioneClient->clientFd,
                    &(datiGiocatore->budgetAttuale), sizeof (int));
            //invia il numero di perdenti al client
            Pthread_mutex_lock(&analisiSessionePuntata.mutex);
            Write(datiGiocatore->datiConnessioneClient->clientFd,
                    &analisiSessionePuntata.numeroPerdenti, sizeof (int));
            Pthread_mutex_unlock(&analisiSessionePuntata.mutex);
        } else if (datiGiocatore->vincitore == 0 || datiGiocatore->vincitore == 2) {
            //perso
            Write(datiGiocatore->datiConnessioneClient->clientFd,
                    &(datiGiocatore->vincitore), sizeof (int));
            Write(datiGiocatore->datiConnessioneClient->clientFd,
                    &(datiGiocatore->budgetAttuale), sizeof (int));
            //invia il numero dei vincitori
            Pthread_mutex_lock(&analisiSessionePuntata.mutex);
            numeroVincitori = analisiSessionePuntata.numeroVincitori;
            Pthread_mutex_unlock(&analisiSessionePuntata.mutex);
            Write(datiGiocatore->datiConnessioneClient->clientFd,
                    &numeroVincitori, sizeof (int));

            //per ogni vincitore, invia indirizzo IP e porta congratulazioni al client
            Pthread_mutex_lock(&analisiSessionePuntata.mutex);
            vincitore_t *temp = (vincitore_t *) analisiSessionePuntata.elencoVincitori.head;
            while (temp != NULL) {
                //scrivere indirizzo ip giocatore[i]
                write(datiGiocatore->datiConnessioneClient->clientFd,
                        inet_ntoa(temp->indirizzoIp.sin_addr), IP_ADDRESS_LENGTH);
                //scrivere porta giocatore[i]
                int tempPort = htons(temp->portaMessaggiCongratulazioni);
                write(datiGiocatore->datiConnessioneClient->clientFd,
                        &tempPort, sizeof (in_port_t));
                
                temp = (vincitore_t *) temp->next;
            }
            Pthread_mutex_unlock(&analisiSessionePuntata.mutex);
        }


        if (datiGiocatore->vincitore == 2) {
            //se ho perso tutti i soldi, eliminarmi dal gioco

            printf("[Player] Ho perso\n");
            sessioneGiocoCorrente.giocatoriConnessi--;
            queue_remove(&(sessioneGiocoCorrente.elencoGiocatori),
                    (node *) datiGiocatore);
            Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);
            break;
        }
        Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);
    }
    free(datiGiocatore);
    free(argomentoGestorePuntate);
    pthread_exit(NULL);
}

void *gestorePuntateGiocatore(void *arg) {
    argomento_gestore_puntate_t *argomento = (argomento_gestore_puntate_t *) arg;
    char stringaPuntata[10];
    puntata_t *singolaPuntata;
    int tipoPuntata;
    int sommaPuntata;

    //invia il messaggio di =puntate aperte= al client
    Write(argomento->clientFd, messaggioPuntateAperte, lenMessaggioPuntateAperte);

    bzero(stringaPuntata, sizeof (stringaPuntata));
    while (1) {
        /*
         * riceve una stringa dal client del tipo "int tipo:int somma" dove:
         * tipo == -1 significa puntata sui dispari
         * tipo == -2 significa puntata sui pari
         * tipo >= 0 rappresenta il numero puntato
         * somma rappresenta la somma puntata
         */
        Read(argomento->clientFd, &tipoPuntata, sizeof (int));
        Read(argomento->clientFd, &sommaPuntata, sizeof (int));

        singolaPuntata = (puntata_t *) Malloc(sizeof (puntata_t));
        singolaPuntata->next = NULL;
        singolaPuntata->tipoPuntata = tipoPuntata;
        singolaPuntata->sommaPuntata = sommaPuntata;
        singolaPuntata->numeroPuntato = tipoPuntata;

        queue_put(argomento->listaPuntatePrivata, (node *) singolaPuntata);
        singolaPuntata = NULL;
    }
    pthread_exit(NULL);
}