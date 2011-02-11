//client.c *

#include "../common/common_header.h"
#include "client_header.h"



void *lettorePuntate(void *arg);

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
    printf("Indirizzo server: %s\n", serverAddress);
    /* porta del server */
    serverPort = atoi(argv[2]);
    printf("Porta server: %d\n", serverPort);
    /* nickname utente */
    strcpy(nickname, argv[3]);
    printf("Nickname: %s\n", nickname);
    /* somma di denaro allocata */
    budget = atoi(argv[4]);
    printf("Denaro: %d\n", budget);

    //TODO Controllo errori più forte

    /*
     * apro il socket di ascolto del client
     */
    clientFd = Socket(AF_INET, SOCK_STREAM, 0);
    //non si esegue la bind per far assegnare una porta random
    Listen(clientFd, 5); //TODO quanti ce ne devono stare nella listen?
   

    status = getsockname(clientFd, (struct sockaddr *) &clientData, &clientAddrlen);
    printf("Porta assegnata: %d", ntohs(clientData.sin_port));

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
    if (write(serverFd, nickname, sizeof (nickname)) < 0) {
        perror("Writing nickname on socket");
        abort();
    }

    while (1) {
        flagFinePuntate = -1;
        /* messaggio che le puntate sono aperte*/
        read(serverFd, buf, sizeof ("\n=Puntate aperte=\n")); //TODO check error
        printf("%s", buf);

        /* crea il thread che gestisce l'acquisizione delle puntate*/
        pthread_create(&tidLettorePuntate, NULL, lettorePuntate, (void *) serverFd); //TODO check error
        //ricevi la segnalazione che le puntate sono chiuse
        read(serverFd, &flagFinePuntate, sizeof (int));
        pthread_cancel(tidLettorePuntate);

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
            close(fd[1]);
            //riceve dal figlio tramite fd[0]
            if (flagFinePuntate == 1) {
                //------------//TODO check errors-----------
                //buffer più grande
                read(fd[0], &lenBufRisultato, sizeof (size_t));
                read(fd[0], bufRisultato, lenBufRisultato);
                //il padre stampa a video il messaggio di congratulazione
                printf("%s\n", bufRisultato);
                //-----------------------
            }
            wait(NULL); //TODO check error

        } else {
            //figlio
            close(fd[0]);
            //manda al padre tramite fd[1]

            //(pipe) da qua già nel figlio...
            if (flagFinePuntate == 1) {
                //ho vinto
                //leggo numero di perdenti
                read(serverFd, &numeroPerdenti, sizeof (int));

                printf("Ho vinto!!\n");
                printf("Devo aspettarmi %d messaggi di congratulazioni\n", numeroPerdenti);

                //deve accettare numPerdenti messaggi sul socket
                while (numeroPerdenti > 0) {
                    //accept
                    perdenteFd = accept(clientFd, NULL, NULL); //TODO check errors
                    //read(sul clientFd,"nickname si congratula");
                    read(perdenteFd, &lenBufCongratulazioni, sizeof (size_t));
                    read(perdenteFd, bufCongratulazioni, lenBufCongratulazioni);
                    //concatenare in un buffer risultato tutti i messaggi
                    strcat(bufRisultato, bufCongratulazioni);
                    strcat(bufRisultato, "\n");
                    close(perdenteFd);
                    numeroPerdenti--;
                }
                //printf("%s\n", bufCongratulazioni);
                //non stampa più bufCongratulazioni ma lo manda al padre che lo stamperà
                //-----------------------
                //comunica al padre tramite pipe il messaggio di congratulazione
                lenBufRisultato = sizeof (bufRisultato);
                write(fd[1], &lenBufRisultato, sizeof (size_t)); //TODO check error
                write(fd[1], bufRisultato, sizeof (bufRisultato)); //TODO check error
                //-----------------------

            } else if (flagFinePuntate == 0) {
                //ho perso

                //deve ricevere numvincitori
                read(serverFd, &numeroVincitori, sizeof (int));
                printf("Ho perso!!!\n");
                printf("Devo mandare %d messaggi di congratulazioni\n", numeroVincitori);
                while (numeroVincitori > 0) {
                    //legge indirizzo IP e porta di ogni vincitore
                    read(serverFd, buf, IP_ADDRESS_LENGTH);
                    read(serverFd, &tempPort, sizeof (in_port_t));
                    printf("%s:%d\n", buf, tempPort);

                    //apre socket
                    vincitoreFd = Socket(AF_INET, SOCK_STREAM, 0); //TODO check error
                    //dati di connessione del vincitore
                    bzero(&vincitoreData, sizeof (vincitoreData));
                    vincitoreData.sin_family = AF_INET;
                    vincitoreData.sin_port = htons(tempPort);
                    inet_aton(buf, &vincitoreData.sin_addr);


                    status = connect(vincitoreFd, (struct sockaddr *) &vincitoreData, sizeof (vincitoreData)); //TODO check error
                    //scrive sul socket
                    bzero(&bufCongratulazioni[0], sizeof (bufCongratulazioni));
                    strcpy(bufCongratulazioni, nickname);
                    strcat(bufCongratulazioni, " si congratula.");
                    lenBufCongratulazioni = sizeof (bufCongratulazioni);

                    write(vincitoreFd, &lenBufCongratulazioni, sizeof (size_t)); //TODO check error
                    write(vincitoreFd, bufCongratulazioni, sizeof (bufCongratulazioni)); //TODO check error
                    //chiude socket
                    close(vincitoreFd); //TODO check error


                    numeroVincitori--;
                }
                //deve leggere tutti gli ip e le porte dei vincitori
                //deve inviare numVincitori messaggi sui socket
            } else {
                //non dovrebbe mai arrivare qui, nel caso, termina
                abort();
            }
            //gestisci messaggi

            exit(1);
        }//fine figlio
        //fine pipe

        //qui finisce il figlio della Pipe
    }//while(1)
    close(serverFd);
    pthread_exit(NULL);
}

void *lettorePuntate(void *arg) {
    ssize_t bytesRead;
    char puntata[20];
    int sommaPuntata;
    int tipoPuntata;
    int numeroPuntato;
    int serverFd = (int) arg;

    while ((bytesRead = read(STDIN_FILENO, puntata, MAXBUF)) > 0) {
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

            if (write(serverFd, &tipoPuntata, sizeof (int)) < 0) {
                perror("Writing money on socket");
                abort();
            }
            if (write(serverFd, &sommaPuntata, sizeof (int)) < 0) {
                perror("Writing money on socket");
                abort();
            }
        }
    }
    pthread_exit(NULL);
}