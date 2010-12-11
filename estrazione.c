/* estrazione.c
 *
 * TODO inserire descrizione file
 */
#define DEBUG 1


#include "common_header.h"
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
	srand(time(NULL));

	/* converto i parametri passati a interi */
	serverPort = atoi(argv[1]);
	gameInterval = atoi(argv[2]);

	printf("La giocata durerà %d secondi\n", gameInterval);

	/* creo il thread CROUPIER */
	status = pthread_create(&croupierTid, NULL, croupier, (void *) gameInterval);
	if (status != 0) {
		err_abort(status, "Creazione del thread croupier");
	}

	sockfd = open_socket(self, serverPort);

	while (1) {
		/* accetta connessioni dai client */
		lenIndirizzoClient = sizeof (indirizzoClient);
		clientFd = accept(sockfd, (struct sockaddr *) &indirizzoClient, &lenIndirizzoClient);
		if (clientFd < 0) {
			err_abort(errno, "Error accepting connection");
		}
		/* inserisco informazioni sul client da inviare al thread player */
		datiConnessioneClient = NULL;
		datiConnessioneClient = malloc(sizeof (client_t));
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


//TODO inserire descrizioni e nomi significativi per le variabili globali
//pthread_mutex_t puntate_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t puntate_aperte = PTHREAD_COND_INITIALIZER;
//pthread_cond_t croupier_cond = PTHREAD_COND_INITIALIZER;
#ifndef DEBUG
/* Crea 10 player thread */
for (j = 0; j < 10; j++) {
	status = pthread_create(&playerTid, NULL, player, (void *) j);
	if (status != 0) {
		err_abort(status, "Creazione thread");
	}
}
#endif