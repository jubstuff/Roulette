#include "common_header.h"
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
	ssize_t bytesRead;
	size_t nicknameLen;
	struct sockaddr_in clientData;
	int flag = 1; //avvisa il client che le puntate sono chiuse
	/*
	 * Quando è già in atto una puntata, non si possono connettere nuovi giocatori
	 * Aspetta che le puntate siano chiuse per connettersi
	 */
	pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
	while (sessionePuntateCorrente.stato == 1) {
		pthread_cond_wait(&sessionePuntateCorrente.chiuse, &sessionePuntateCorrente.mutex);
	}
	pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

	/*
	 * Recupera dati dal client e inserisci un nuovo giocatore nella lista
	 */
	datiGiocatore = (player_t *) malloc(sizeof (player_t)); //TODO check error

	datiGiocatore->datiConnessioneClient = (client_t *) arg;
	//TODO error check sulle read
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, &datiGiocatore->portaMessaggiCongratulazioni, sizeof (in_port_t));
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, &datiGiocatore->budgetAttuale, sizeof (int));
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, &nicknameLen, sizeof (size_t));
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, datiGiocatore->nickname, nicknameLen);

	datiGiocatore->portaMessaggiCongratulazioni = ntohs(datiGiocatore->portaMessaggiCongratulazioni);
	datiGiocatore->budgetPrecedente = 0;
	datiGiocatore->vincitore = 0;

	printf("====Dati Giocatore====\n");
	printf("Nickname: %s\n", datiGiocatore->nickname);
	printf("Budget Iniziale: %d\n", datiGiocatore->budgetAttuale);
	printf("Porta Congratulazioni: %d\n\n", datiGiocatore->portaMessaggiCongratulazioni);

	//write(datiGiocatore->datiConnessioneClient->clientFd, "[server] sei connesso", sizeof("[server] sei connesso"));

	pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
	queue_put(&sessioneGiocoCorrente.elencoGiocatori, (node *) datiGiocatore);
	sessioneGiocoCorrente.giocatoriConnessi++;
	/*
	 * Se questo thread è il primo giocatore connesso, avvisa il croupier che 
	 * può iniziare la giocata
	 */
	if (sessioneGiocoCorrente.giocatoriConnessi == 1) {
		pthread_cond_signal(&sessioneGiocoCorrente.attesaAlmenoUnGiocatore); //TODO check error
	}
	pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

	/*
	 * Prepara l'argomento da passare al thread gestore delle puntate
	 */
	argomentoGestorePuntate = (argomento_gestore_puntate_t *) malloc(sizeof (argomento_gestore_puntate_t)); //TODO check error
	queue_init(&listaPuntatePrivata);
	argomentoGestorePuntate->listaPuntatePrivata = &listaPuntatePrivata;
	argomentoGestorePuntate->clientFd = datiGiocatore->datiConnessioneClient->clientFd;


	while (1) {
		/*
		 * Aspetta che il croupier apra le puntate
		 */
		printf("[Player] Aspetto l'apertura delle puntate\n");
		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		while (sessionePuntateCorrente.stato == 0) {
			pthread_cond_wait(&sessionePuntateCorrente.aperte, &sessionePuntateCorrente.mutex); //TODO check error
		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

		printf("[Player] Creo il gestore delle puntate\n");
		pthread_create(&tidGestorePuntateGiocatore, NULL, gestorePuntateGiocatore, (void *) argomentoGestorePuntate); //TODO check error
		/*
		 * Aspetta che il croupier chiuda le puntate
		 */
		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		while (sessionePuntateCorrente.stato == 1) {
			pthread_cond_wait(&sessionePuntateCorrente.chiuse, &sessionePuntateCorrente.mutex); //TODO check error

		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

		pthread_cancel(tidGestorePuntateGiocatore); //TODO check error
		/*
		 * collegare pacchetto di puntate alla lista del giocatore
		 */
		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
		/* aggiunge la lista recuperata dal gestore puntate al nodo relativo al
		 * giocatore nella lista globale */
		datiGiocatore->elencoPuntate.head = listaPuntatePrivata.head;
		sessioneGiocoCorrente.giocatoriChePuntano--;
		/*
		 * Se tutti i giocatori hanno collegato il proprio pacchetto di puntate
		 * risveglia il croupier
		 */
		if (sessioneGiocoCorrente.giocatoriChePuntano == 0) {
			pthread_cond_signal(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate); //TODO check error
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

		queue_init(&listaPuntatePrivata);
		queue_init(argomentoGestorePuntate->listaPuntatePrivata);

		//* Aspettare che il croupier gestisca le puntate
		//* Quando segnalato, far partire la gestione dei messaggi tra client

		pthread_mutex_lock(&analisiSessionePuntata.mutex); //TODO check error
		while (analisiSessionePuntata.stato == 0) {
			printf("[player] Aspetto che termini la gestione delle puntaten\n");
			pthread_cond_wait(&analisiSessionePuntata.attesaMessaggi, &analisiSessionePuntata.mutex); //TODO check error
		}
		pthread_mutex_unlock(&analisiSessionePuntata.mutex); //TODO check error

		//=gestione messaggi tra client

		//connetterci al socket del client e inviare messaggio di chiusura puntate

		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error

		if (datiGiocatore->vincitore == 1) {
			//invia 1 per dire che ha vinto
			flag = 1;
			write(datiGiocatore->datiConnessioneClient->clientFd, &flag, sizeof (int)); //TODO check error
			//invia il numero di perdenti al client
			//pthread_mutex_lock(&analisiSessionePuntata.mutex); //TODO check error
			//write(datiGiocatore->datiConnessioneClient->clientFd, &analisiSessionePuntata.numeroPerdenti, sizeof (int)); //TODO check error
			//pthread_mutex_unlock(&analisiSessionePuntata.mutex); //TODO check error
			//azzero il flag per la prossima puntata
			datiGiocatore->vincitore = 0;
		} else {
			//invia 0 per dire che ha perso
			flag = 0;
			write(datiGiocatore->datiConnessioneClient->clientFd, &flag, sizeof (int));
			//invia il numero dei vincitori
			//per ogni vincitore, invia indirizzo IP e porta congratulazioni al client

		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

	}
	free(datiGiocatore);
	free(argomentoGestorePuntate);
	pthread_exit(NULL);
}

void *gestorePuntateGiocatore(void *arg) {
	argomento_gestore_puntate_t *argomento = (argomento_gestore_puntate_t *) arg;
	ssize_t bytes_read;
	char stringaPuntata[10];
	puntata_t *singolaPuntata;
	int tipoPuntata;
	int sommaPuntata;


	bzero(stringaPuntata, sizeof (stringaPuntata));
	while (1) {
		/* riceve una stringa dal client del tipo "int tipo:int somma" dove:
		 * tipo == -1 significa puntata sui dispari
		 * tipo == -2 significa puntata sui pari
		 * tipo >= 0 rappresenta il numero puntato
		 * somma rappresenta la somma puntata
		 */
		bytes_read = read(argomento->clientFd, &tipoPuntata, sizeof (int));
		bytes_read = read(argomento->clientFd, &sommaPuntata, sizeof (int));

		singolaPuntata = (puntata_t *) malloc(sizeof (puntata_t)); //TODO check error
		singolaPuntata->next = NULL;
		singolaPuntata->tipoPuntata = tipoPuntata;
		singolaPuntata->sommaPuntata = sommaPuntata;
		singolaPuntata->numeroPuntato = tipoPuntata;
		/*
			
				printf("Il tipo puntata è %d\n", singolaPuntata->tipoPuntata);
				printf("La somma puntata è %d\n", singolaPuntata->sommaPuntata);
				if(singolaPuntata->numeroPuntato >= 0) {
					printf("Il numero puntato è %d\n", singolaPuntata->numeroPuntato);
				}
		 */

		queue_put(argomento->listaPuntatePrivata, (node *) singolaPuntata);
		singolaPuntata = NULL;
	}
	pthread_exit(NULL);
}