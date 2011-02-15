/**
 * @file   client_header.c
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include tutte le definizioni per il client della Roulette
 *
 */
#include "client_header.h"
#include <stdio.h>
#include <ctype.h>

void *lettorePuntate(void *arg) {
    ssize_t bytesRead;
    char puntata[20];
    int sommaPuntata;
    int tipoPuntata;
    int numeroPuntato;
    int serverFd = (int) arg;
    char prompt[] = "Puntata?>";

    Write(STDIN_FILENO, prompt, sizeof (prompt));
    while ((bytesRead = Read(STDIN_FILENO, puntata, MAXBUF)) > 0) {
        puntata[bytesRead - 1] = '\0';
        if ((strcmp(puntata, "exit") == 0)) {
            //TODO rimuovere?
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
        Write(STDIN_FILENO, prompt, sizeof (prompt));
    }
    pthread_exit(NULL);
}

int parse_bet(char *puntataStr, int *sommaPuntata, int *tipoPuntata, int *numeroPuntato) {
    /* FIXME non c'è controllo sull'overflow dell'int dei soldi.
     * Non dovrebbe essere un grosso problema in ogni caso, perché
     * il controllo sul valore massimo della puntata si deve comunque
     * fare, anche se non in questa funzione */
    int error_flag = 0;
    char bet_type;

    *numeroPuntato = NO_NUMBER;

    if ((sscanf(puntataStr, "%d:%d", numeroPuntato, sommaPuntata)) == 2) {
        /* è stata inserita una puntata corretta del tipo <X>:<N> */
        if (*numeroPuntato >= 0 && *numeroPuntato <= 36) {
            //OK
            *tipoPuntata = *numeroPuntato;
        } else {
            //puntato un numero non corretto
            error_flag = ENUMRANGE;
        }
    } else if ((sscanf(puntataStr, "%c:%d", &bet_type, sommaPuntata)) == 2) {
        /* è stata inserita una puntata del tipo char:<N>*/
        bet_type = toupper(bet_type);
        if (bet_type == 'P') {
            //caso P:<N>
            *tipoPuntata = -2;
        } else if (bet_type == 'D') {
            //caso D:<N>
            *tipoPuntata = -1;
        } else {
            //carattere non valido
            error_flag = ECHARINV;
        }
    } else {
        //sintassi puntata non valida
        error_flag = EBADSYNTAX;
    }
    return error_flag;
}

char *tipoPuntataTestuale(int tipo) {
    if (tipo == -1) {
        return "Dispari";
    } else if (tipo == -2) {
        return "Pari";
    } else if (tipo >= 0 && tipo <= 36) {
        return "Numero";
    } else {
        return "Puntata non valida";
    }
}

void gestisciMessaggiVittoria(int serverFd, int clientFd, char *bufRisultato) {
    int perdenteFd;
    size_t lenBufCongratulazioni;
    char bufCongratulazioni[100];
    int numeroPerdenti;

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
        (numeroPerdenti)--;
    }

}

void gestisciMessaggiPerdita(int serverFd, char *nickname) {
    //ho perso
    int vincitoreFd;
    struct sockaddr_in vincitoreData;
    in_port_t tempPort;
    char buf[100];
    size_t lenBufCongratulazioni;
    char bufCongratulazioni[100];
    int numeroVincitori;

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


        Connect(vincitoreFd, (struct sockaddr *) &vincitoreData,
                sizeof (vincitoreData));
        //scrive sul socket
        bzero(&bufCongratulazioni[0], sizeof (bufCongratulazioni));
        strcpy(bufCongratulazioni, nickname);
        strcat(bufCongratulazioni, " si congratula.");
        lenBufCongratulazioni = sizeof (bufCongratulazioni);

        Write(vincitoreFd, &lenBufCongratulazioni, sizeof (size_t));
        Write(vincitoreFd, bufCongratulazioni, sizeof (bufCongratulazioni));
        //chiude socket
        Close(vincitoreFd);

        (numeroVincitori)--;
    }
}
