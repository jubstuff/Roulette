//client.c *

#include "common_header.h"
#include "client_header.h"

#define IP_ADDRESS_LENGTH 15

int main(int argc, char **argv) {
	int serverFd;
	int clientFd;
	int status;

	struct sockaddr_in serverData;
	struct sockaddr_in clientData;
	socklen_t clientAddrlen;
	char serverAddress[IP_ADDRESS_LENGTH];
	in_port_t serverPort;
	in_port_t congratPort;
	char nickname[NICK_LENGTH];
	size_t nicknameLen;
	int budget;
	ssize_t bytesRead;
	char puntata[20];
	int sommaPuntata;
	int tipoPuntata;
	int numeroPuntato;


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
	clientFd = socket(AF_INET, SOCK_STREAM, 0);
	if (clientFd < 0) {
		abort();
	}

	status = listen(clientFd, 5); //TODO quanti ce ne devono stare nella listen?
	if (status != 0) {
		abort();
	}

	status = getsockname(clientFd, (struct sockaddr *) &clientData, &clientAddrlen);


	/*
	 * Creazione socket e connessione al server
	 */
	/* creo il socket */
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) {
		err_abort(errno, "Errore nella connessione: riavvia il programma.");
	}

	/* inizializzo la struct sockaddr_in */
	bzero(&serverData, sizeof (serverData));
	serverData.sin_family = AF_INET;
	serverData.sin_port = htons(serverPort);
	inet_aton(serverAddress, &serverData.sin_addr);

	/* mi connetto al server */
	status = connect(serverFd, (struct sockaddr *) &serverData, sizeof (serverData));
	if (status != 0) {
		err_abort(errno, "Errore nella connessione: riavvia il programma.");
	}
	printf("Connesso al server.\n");

	/* invio porta di congratulazioni al server */
	congratPort = htons(clientData.sin_port);
	if (write(serverFd, &congratPort, sizeof (in_port_t)) < 0) {
		perror("Writing server port on socket");
		abort();
	}
	/* invio budget al server */
	if (write(serverFd, &budget, sizeof (int)) < 0) {
		perror("Writing money on socket");
		abort();
	}
	/* invio la lunghezza del nickname al server */
	nicknameLen = sizeof (nickname);
	if (write(serverFd, (void *) (&nicknameLen), sizeof (size_t)) < 0) {
		perror("Writing nickname on socket");
		abort();
	}
	/* invio il nickname al server */
	if (write(serverFd, nickname, sizeof (nickname)) < 0) {
		perror("Writing nickname on socket");
		abort();
	}

	while ((bytesRead = read(STDIN_FILENO, puntata, MAXBUF)) > 0) {
		puntata[bytesRead - 1] = '\0';
		if ((strcmp(puntata, "exit") == 0)) {
			printf("Esco\n");
			exit(1);
		}
		if (!parse_bet(puntata, &sommaPuntata, &tipoPuntata, &numeroPuntato)) {
			printf("Sono stati puntati %d€\n", sommaPuntata);
			printf("Il tipo puntata è %d\n", tipoPuntata);
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
	close(serverFd);
	return 0;
}
