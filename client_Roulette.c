//Client ROULETTE 2010 
#include "common_header.h"

char *gestioneGiocata(void) {
	int somma, numero;
	char *strServer, scelta;

	printf("Inserisci la somma che vuoi puntare: ");
	//Anche se non mi piace lo scanf
	scanf("%d", &somma);

	//Il client scegli di puntare su PARI/DISPARI o su un numero da 0 a 36
	printf("Vuoi giocare su pari/dispari o su un numero: ")
	printf("Premi 1 per PARI/DISPARI\n");
	printf("Premi 2 per NUMERO\n");
	scanf("%s", &scelta);
	switch (scelta) {
		case '1':
			do {
				printf("Inserisci P per PARI oppure D per DISPARI: \n");
				scanf("%d", &scelta);
				scelta = toupper(scelta);
			} while(scelta != 'P' && scelta != 'D');
			break;
		case '2':
			do {
				printf("Inserisci il numero su cui vuoi puntare (0-36): ");
				scanf("%s", &scelta);
				numero = atoi(scelta);
			} while ((numero < 0) && (numero > 36));
			
			break;
		default: 
			break; //aggiungi sempre una clausola di default
	}
	//Concatenzione delle stringhe
	strServer = scelta;
	strcat((strServer, ":"), somma);
	return(strServer);
}

int main(int argc, char* argv[]) {
	struct sockaddr_in sockaddr_server;
	int fdSocket;
	char *server_address;
	in_port_t server_port;
	in_port_t congrat_port;
	char *nickname;
	int money;
	char buf[MAXBUF];
	char *sintassi_server;
	ssize_t bytes_read;

	//controllo il numero di argomenti
	if (argc != 6) {
		printf("Utilizzo: %s <indirizzo server> <numero porta> <nickname> \
			 <somma allocata> <porta congratulazioni>\n", argv[0]);
		exit(1);
	}

	//indirizzo del server
	server_address = argv[1];
	printf("Indirizzo server: %s\n", server_address);

	//porta del server
	server_port = atoi(argv[2]);
	printf("Porta server: %d\n", server_port);

	//nickname utente
	nickname = argv[3];
	printf("Nickname: %s\n", nickname);

	//somma di denaro allocata
	money = atoi(argv[4]);
	printf("Denaro: %d\n", money);

	//porta per le congratulazioni //FIXME dobbiamo farla scegliere al SO
	congrat_port = atoi(argv[5]);
	printf("Porta congratulazioni: %d\n", congrat_port);

	//Crea il socket
	fdSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (fdSocket > 0)
		printf("Socket creato\n");
	else {
		printf("Impossibile creare il socket\n");
		exit(1);
	}

	//Inizializzazione della struttura sockaddr_in

	//TODO: Manca forse questa istruzione "bzero(&server_sockaddr, sizeof(server_sockaddr));" che non ho messo perchè non l'ho capita

	//Il primo campo indica la famiglia del protocollo, AF_INET per quelli di rete
	sockaddr_server.sin_family = AF_INET;
	//Il secondo campo indica in numero di porta del servizio di rete
	sockaddr_server.sin_port = htons(server_port);
	//Il terzo campo contiene una struttura che identifica l'IP del servizio di rete
	inet_aton(server_address, &sockaddr_server.sin_addr);

	//Connette all'indirizzo specificato e controlla se c'è la connessione
	if (connect(fdSocket, (struct sockaddr *) &sockaddr_server, sizeof (sockaddr_server)) < 0) {
		printf("Impossibile connettersi\n");
		exit(1);
	} else
		printf("Connesso\n");

	/* Spedisce al server i messaggi contenenti:
	 * - porta di congratulazioni
	 * - soldi
	 * - nickname
	 */
	//invio porta al server
	if (write(fdSocket, &congrat_port, sizeof (server_port)) < 0) {
		perror("Writing server port on socket");
		abort();
	}
	//invio soldi al server
	if (write(fdSocket, &money, sizeof (money)) < 0) {
		perror("Writing money on socket");
		abort();
	}
	//invio il nickname al server
	if (write(fdSocket, nickname, sizeof (nickname)) < 0) {
		perror("Writing nickname on socket");
		abort();
	}

	while ((bytes_read = read(STDIN_FILENO, buf, MAXBUF)) > 0) {
		buf[bytes_read - 1] = '\0';
		if ((strcmp(buf, "exit") == 0)) {
			printf("Esco\n");
			exit(1);
		}
		//TODO: Dovrebbe essere questa la parte sulla puntata da parte del client
		//TODO: Non ricordo se devo usate strcp o va bene l'assegnazione
		sintassi_server = gestioneGiocata();
		write(fdSocket, sintassi_server, sizeof (sintassi_server));

	}

	//Chiude il socket
	close(fdSocket);
	return 0;
}