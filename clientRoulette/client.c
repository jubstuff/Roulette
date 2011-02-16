/**
 * @file   client.c
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Main function per il client della Roulette e lettore puntate
 *
 */

#include "../common/common_header.h"
#include "client_header.h"

int main(int argc, char **argv) {
    int serverFd;
    int clientFd;

    char bufRisultato[MAXBUF];
    size_t lenBufRisultato;

    struct sockaddr_in serverData;
    struct sockaddr_in clientData;
    //importante per il corretto funzionamento della getsockaddrname
    socklen_t clientAddrlen = sizeof (clientData);

    char serverAddress[IP_ADDRESS_LENGTH];
    in_port_t serverPort;
    in_port_t congratPort;

    char nickname[NICK_LENGTH];
    size_t nicknameLen;
    int budget;

    pthread_t tidLettorePuntate;
    char buf[100];
    int flagFinePuntate = -1;

    int fd[2];
    int pid;


    /* controllo numero di argomenti */
    if (argc != 5) {
        printf("Utilizzo: %s <indirizzo server> <numero porta> <nickname> <somma allocata>\n",
                argv[0]);
        exit(1);
    }

    /* indirizzo del server */
    strcpy(serverAddress, argv[1]);
    /* porta del server */
    serverPort = atoi(argv[2]);
    /* nickname utente */
    strcpy(nickname, argv[3]);
    /* somma di denaro allocata */
    budget = atoi(argv[4]);

    if (budget > LIMITE_BUDGET) {
        err_abort(0, "Limite budget superato. Imposta un budget minore");
    }

    printf("Indirizzo server: %s\n", serverAddress);
    printf("Porta server: %d\n", serverPort);
    printf("Nickname: %s\n", nickname);
    printf("Denaro: %d\n", budget);

    /*
     * apro il socket di ascolto del client
     */
    clientFd = Socket(AF_INET, SOCK_STREAM, 0);
    //non si esegue la bind per far assegnare una porta random
    Listen(clientFd, 5);
    Getsockname(clientFd, (struct sockaddr *) &clientData, &clientAddrlen);
    printf("Porta assegnata: %d\n", ntohs(clientData.sin_port));

    /*
     * Creazione socket e connessione al server
     */
    serverFd = Socket(AF_INET, SOCK_STREAM, 0);


    /* inizializzo la struct sockaddr_in */
    bzero(&serverData, sizeof (serverData));
    serverData.sin_family = AF_INET;
    serverData.sin_port = htons(serverPort);
    inet_aton(serverAddress, &serverData.sin_addr);

    /* mi connetto al server */
    Connect(serverFd, (struct sockaddr *) &serverData, sizeof (serverData));
    printf("Connesso al server.\n");

    /* invio porta di congratulazioni al server */
    congratPort = htons(clientData.sin_port);
    Write(serverFd, &congratPort, sizeof (in_port_t));

    /* invio budget al server */
    Write(serverFd, &budget, sizeof (int));

    /* invio la lunghezza del nickname al server */
    nicknameLen = sizeof (nickname);
    Write(serverFd, (void *) (&nicknameLen), sizeof (size_t));

    /* invio il nickname al server */
    Write(serverFd, nickname, sizeof (nickname));

    while (1) {
        flagFinePuntate = -1;
        /* messaggio che le puntate sono aperte*/
        Read(serverFd, buf, sizeof ("\n=Puntate aperte=\n"));
        printf("%s", buf);

        /* crea il thread che gestisce l'acquisizione delle puntate*/
        Pthread_create(&tidLettorePuntate, NULL, lettorePuntate, (void *) serverFd);
        //ricevi la segnalazione che le puntate sono chiuse
        Read(serverFd, &flagFinePuntate, sizeof (int));
        Pthread_cancel(tidLettorePuntate);
        //=TODO AGGIUNTE DOPO CHECK FINALE
        //Read(serverFd, &numeroEstratto, sizeof (int));
        Read(serverFd, &budget, sizeof (int));
        
        //=FINE AGGIUNTE DOPO CHECK FINALE
        //azzero le stringhe
        bzero(&bufRisultato[0], sizeof (bufRisultato));

        //creazione pipe
        Pipe(fd);

        if ((pid = fork()) < 0) {
            err_abort(errno, "Errore nella fork");
        } else if (pid > 0) {
            //padre
            Close(fd[1]);
            //riceve dal figlio tramite fd[0]
            if (flagFinePuntate == 1) {
                //se ho vinto, ricevo i messaggi di congratulazioni
                Read(fd[0], &lenBufRisultato, sizeof (size_t));
                Read(fd[0], bufRisultato, lenBufRisultato);
                //il padre stampa a video il messaggio di congratulazione
                printf("%s", bufRisultato);

            }
            wait(NULL); //TODO check error

        } else {
            //figlio
            Close(fd[0]);
            if (flagFinePuntate == 1) {
                //ho vinto
                gestisciMessaggiVittoria(serverFd, clientFd, bufRisultato);
                printf("Budget Attuale: %d\n", budget);
                //invio il risultato al padre, tramite fd[1]
                strcat(bufRisultato, "\0");
                lenBufRisultato = sizeof (bufRisultato);
                Write(fd[1], &lenBufRisultato, sizeof (size_t));
                Write(fd[1], bufRisultato, sizeof (bufRisultato));
            } else if (flagFinePuntate == 0 || flagFinePuntate == 2) {
                //ho perso
                gestisciMessaggiPerdita(serverFd, nickname);
                printf("Budget Attuale: %d\n", budget);
            } else {
                //non dovrebbe mai arrivare qui, nel caso, termina
                abort();
            }
            exit(1);
        }//fine figlio
        if (flagFinePuntate == 2) {
            //se ho finito i soldi, termina
            break;
        }
    }//while(1)
    
    //chiudo i socket 
    Close(serverFd);
    Close(clientFd);
    pthread_exit(NULL);
}
