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

void parse_bet(char *bet) {
	/* FIXME non c'è controllo sull'overflow dell'int dei soldi.
	 * Non dovrebbe essere un grosso problema in ogni caso, perché
	 * il controllo sul valore massimo della puntata si deve comunque
	 * fare, anche se non in questa funzione */
	int money_bet; //somma puntata
	char bet_type; //tipo di puntata (Pari/Dispari)
	int number_bet;//numero puntato (0-36)

	if ( (sscanf(bet, "%d:%d", &number_bet, &money_bet)) == 2) {
		/* è stata inserita una puntata corretta del tipo <X>:<N> */
		if(number_bet >= 0 && number_bet <= 36) {
			printf("Numero\n");
			printf("Sono stati puntati %d€ sul %d\n", money_bet, number_bet);
		} else {
			//puntato un numero non nel range 0-36
			printf("Errore: bisogna puntare un numero 0 <= N <= 36\n");
		}
	}
	else if ( (sscanf(bet, "%c:%d", &bet_type, &money_bet)) == 2) {
		/* è stata inserita una puntata del tipo char:<N> */
		bet_type = toupper(bet_type);
		if (bet_type == 'P') {
			//caso P:<N>
			printf("Pari\n");
			printf("Sono stati puntati %d€ sui pari\n", money_bet);
		}
		else if(bet_type == 'D') {
			//caso D:<N>
			printf("Dispari\n");
			printf("Sono stati puntati %d€ sui dispari\n", money_bet);
		}
		else {
			//carattere non valido
			printf("Puntata non valida: il carattere è errato\n");
		}
	}
	else {
		//sintassi puntata non valida
		printf("Puntata non valida: errore sintassi\n");
	}
}