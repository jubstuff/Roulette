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
    int perdenteFd;
    int vincitoreFd;
    int status;
    size_t lenBufCongratulazioni;
    char bufRisultato[MAXBUF];
    size_t lenBufRisultato;

    struct sockaddr_in serverData;
    struct sockaddr_in clientData;
    struct sockaddr_in vincitoreData;
    socklen_t clientAddrlen = sizeof (clientData); //importante per il corretto funzionamento della getsockaddrname
    char serverAddress[IP_ADDRESS_LENGTH];
    in_port_t serverPort;
    in_port_t congratPort;
    char nickname[NICK_LENGTH];
    size_t nicknameLen;
    int budget;
    pthread_t tidLettorePuntate;
    char buf[100];
    char bufCongratulazioni[100];
    in_port_t tempPort;
    int flagFinePuntate = -1;
    int numeroPerdenti; //numero richieste da accettare
    int numeroVincitori; //numero di messaggi da inviare
    int fd[2];
    int pid;


    /* controllo numero di argomenti */
    if (argc != 5) {
        printf("Utilizzo: %s <indirizzo server> <numero porta> <nickname> <somma allocata>\n", argv[0]);
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

    if(budget > LIMITE_BUDGET) {
        err_abort(0, "Limite budget superato. Imposta un budget minore");
    }

    printf("Indirizzo server: %s\n", serverAddress);
    printf("Porta server: %d\n", serverPort);
    printf("Nickname: %s\n", nickname);
    printf("Denaro: %d\n", budget);
    //TODO Controllo errori più forte

    /*
     * apro il socket di ascolto del client
     */
    clientFd = Socket(AF_INET, SOCK_STREAM, 0);
    //non si esegue la bind per far assegnare una porta random
    Listen(clientFd, 5); //TODO quanti ce ne devono stare nella listen?


    status = getsockname(clientFd, (struct sockaddr *) &clientData, &clientAddrlen);
    printf("Porta assegnata: %d\n", ntohs(clientData.sin_port));

    /*
     * Creazione socket e connessione al server
     */
    /* creo il socket */
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

        //azzero le stringhe
        bzero(&bufRisultato[0], sizeof (bufRisultato));
        bzero(&bufCongratulazioni[0], sizeof (bufCongratulazioni));

        //creazione pipe
        if (pipe(fd) < 0) {
            perror("pipe");
            exit(1);
        }
        if ((pid = fork()) < 0) {
            perror("fork");
            exit(1);
        } else if (pid > 0) {
            //padre
            Close(fd[1]);
            //riceve dal figlio tramite fd[0]
            if (flagFinePuntate == 1) {
                //se ho vinto, ricevo i messaggi di congratulazioni
                //buffer più grande
                Read(fd[0], &lenBufRisultato, sizeof (size_t));
                Read(fd[0], bufRisultato, lenBufRisultato);
                //il padre stampa a video il messaggio di congratulazione
                printf("%s", bufRisultato);

            }
            wait(NULL); //TODO check error

        } else {
            //figlio
            Close(fd[0]);
            //manda al padre tramite fd[1]

            //(pipe) da qua già nel figlio...
            if (flagFinePuntate == 1) {
                //ho vinto
                //leggo numero di perdenti
                Read(serverFd, &numeroPerdenti, sizeof (int));

                printf("\nHo vinto!!\n");
                printf("Devo aspettarmi %d messaggi di congratulazioni\n", numeroPerdenti);

                //deve accettare numPerdenti messaggi sul socket
                while (numeroPerdenti > 0) {
                    //accept
                    perdenteFd = Accept(clientFd, NULL, NULL);
                    //read(sul clientFd,"nickname si congratula");
                    Read(perdenteFd, &lenBufCongratulazioni, sizeof (size_t));
                    Read(perdenteFd, bufCongratulazioni, lenBufCongratulazioni);
                    //concatenare in un buffer risultato tutti i messaggi
                    strcat(bufRisultato, bufCongratulazioni);
                    strcat(bufRisultato, "\n");

                    Close(perdenteFd);
                    numeroPerdenti--;
                }
                //printf("%s\n", bufCongratulazioni);
                //non stampa più bufCongratulazioni ma lo manda al padre che lo stamperà
                //-----------------------
                //comunica al padre tramite pipe il messaggio di congratulazione
                strcat(bufRisultato, "\0");
                lenBufRisultato = sizeof (bufRisultato);
                Write(fd[1], &lenBufRisultato, sizeof (size_t));
                Write(fd[1], bufRisultato, sizeof (bufRisultato));
                //TODO controllare se funziona il reset
                bzero(&bufRisultato[0], sizeof (bufRisultato));
                //-----------------------

            } else if (flagFinePuntate == 0 || flagFinePuntate == 2) {
                //ho perso

                //deve ricevere numvincitori
                Read(serverFd, &numeroVincitori, sizeof (int));
                printf("\nHo perso!!!\n");
                printf("Devo mandare %d messaggi di congratulazioni\n", numeroVincitori);
                while (numeroVincitori > 0) {
                    //legge indirizzo IP e porta di ogni vincitore
                    Read(serverFd, buf, IP_ADDRESS_LENGTH);
                    Read(serverFd, &tempPort, sizeof (in_port_t));
                    printf("%s:%d\n", buf, tempPort);

                    //apre socket
                    vincitoreFd = Socket(AF_INET, SOCK_STREAM, 0);
                    //dati di connessione del vincitore
                    bzero(&vincitoreData, sizeof (vincitoreData));
                    vincitoreData.sin_family = AF_INET;
                    vincitoreData.sin_port = htons(tempPort);
                    inet_aton(buf, &vincitoreData.sin_addr);


                    Connect(vincitoreFd, (struct sockaddr *) &vincitoreData, sizeof (vincitoreData));
                    //scrive sul socket
                    bzero(&bufCongratulazioni[0], sizeof (bufCongratulazioni));
                    strcpy(bufCongratulazioni, nickname);
                    strcat(bufCongratulazioni, " si congratula.");
                    lenBufCongratulazioni = sizeof (bufCongratulazioni);

                    Write(vincitoreFd, &lenBufCongratulazioni, sizeof (size_t));
                    Write(vincitoreFd, bufCongratulazioni, sizeof (bufCongratulazioni));
                    //chiude socket
                    Close(vincitoreFd);


                    numeroVincitori--;
                }
                //deve leggere tutti gli ip e le porte dei vincitori
                //deve inviare numVincitori messaggi sui socket
            }
/*
            else if(flagFinePuntate == 2) {
                //ho finito i soldi
                strcpy(bufRisultato, "Hai Perso\n");
                Write(fd[1], &lenBufRisultato, sizeof (size_t));
                Write(fd[1], bufRisultato, sizeof (bufRisultato));
            }
*/
            else {
                //non dovrebbe mai arrivare qui, nel caso, termina
                abort();
            }
            //gestisci messaggi

            exit(1);
        }//fine figlio
        //fine pipe
        if(flagFinePuntate == 2){
            break;
        }
    }//while(1)
    Close(serverFd);
    pthread_exit(NULL);
}

void *lettorePuntate(void *arg) {
    ssize_t bytesRead;
    char puntata[20];
    int sommaPuntata;
    int tipoPuntata;
    int numeroPuntato;
    int serverFd = (int) arg;
    char prompt[] = "Puntata?>";
    Write(STDIN_FILENO, prompt, sizeof(prompt));
    while ((bytesRead = Read(STDIN_FILENO, puntata, MAXBUF)) > 0) {
        puntata[bytesRead - 1] = '\0';
        if ((strcmp(puntata, "exit") == 0)) {
            printf("Esco\n");
            exit(1);
        }
        
        if (!parse_bet(puntata, &sommaPuntata, &tipoPuntata, &numeroPuntato)) {
            printf("Sono stati puntati %d€\n", sommaPuntata);
            printf("Il tipo puntata è %s\n", tipoPuntataTestuale(tipoPuntata));
            if (tipoPuntata >= 0) {
                printf("È stato puntato il numero %d\n", numeroPuntato);
            }

            Write(serverFd, &tipoPuntata, sizeof (int));
            Write(serverFd, &sommaPuntata, sizeof (int));

        } else {
            printf("Puntata non valida, ritenta.\n");
        }
        Write(STDIN_FILENO, prompt, sizeof(prompt));
    }
    pthread_exit(NULL);
}