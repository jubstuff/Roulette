#include "common_header.h"

int num_requests = 0;

/**
 * Stampa sullo standard error un messaggio contenente l'errore, il file che l'ha
 * causato, la riga e la spiegazione.
 * 
 * @param code il codice d'errore (il valore di ritorno per le funzioni pthread,
 *		la variabile errno per le altre syscall)
 * @param text Una stringa che spiega l'errore
 */

void err_abort(int code, char *text) {
	fprintf(stderr, "%s at %s:%d: %s\n", text, __FILE__, __LINE__, strerror(code));
	abort();
}

int open_socket(struct sockaddr_in self, short int server_port) {
	int sockfd;
	int status;
	/* apro il socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		err_abort(errno, "Creazione socket");
	}

	/* preparo la struct con le informazioni del server */
	bzero(&self, sizeof (self));
	/* protocol family in host order */
	self.sin_family = AF_INET;
	self.sin_port = htons(server_port);
	self.sin_addr.s_addr = htonl(INADDR_ANY);

	/* collego il socket */
	status = bind(sockfd, (struct sockaddr *) &self, sizeof (self));
	if (status != 0) {
		err_abort(errno, "Bind socket");
	}
	/* pone il server in ascolto */
	status = listen(sockfd, 5); //TODO quanti ce ne devono stare nella listen?
	if (status != 0) {
		err_abort(errno, "Error in listening to socket");
	}
	return sockfd;
}

void gestisci_puntata(int estratto) {
	puntata_t *puntata;
	while ((puntata = (puntata_t *) queue_get(&(lista_puntate.puntate))) != NULL) {
		fprintf(stdout, "CROUPIER: nella lista delle puntate numero %d di tipo %d con %d€\n",
			puntata->numero, puntata->tipo, puntata->somma_puntata);
		if (estratto == puntata->numero) {
			fprintf(stdout, "Questa puntata vince!!\n");
		}
		free(puntata);
		num_requests--;
	}
}

struct timespec calcola_intervallo(int intervallo) {
	time_t now;
	struct timespec cond_time;
	now = time(NULL);
	cond_time.tv_sec = now + intervallo;
	cond_time.tv_nsec = 0;
	return cond_time;
}