#include "common_header.h"
#include "player.h"

/**
 * FUNZIONE player
 *
 *============================================================================*/
void *player(void *arg) {
	/*Per adesso accettiamo come argomento un numero che identifica il thread.
	 * La versione definitiva dovrà accettare informazioni sul client
	 */
	int num_giocatore = (int) arg;
	player_t *datiGiocatore;
	pthread_t tidGestorePuntateGiocatore;
	queue listaPuntatePrivata;
	argomento_gestore_puntate_t *argomentoGestorePuntate;


	datiGiocatore = (player_t *) malloc(sizeof (player_t)); //TODO check error

	//======================DA ELIMINARE===================================
	//TODO questi dati devono essere presi dal client, ovviamente
	datiGiocatore->budgetAttuale = (rand() % MAX_BUDGET) + 1;
	snprintf(datiGiocatore->nickname, sizeof (datiGiocatore->nickname),
			"%s%d", "Giocatore", num_giocatore);
	//======================DA ELIMINARE===================================

	pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
	queue_put(&sessioneGiocoCorrente.elencoGiocatori, (node *) datiGiocatore);
	sessioneGiocoCorrente.giocatoriConnessi++;
	pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

	argomentoGestorePuntate = (argomento_gestore_puntate_t *) malloc(sizeof (argomento_gestore_puntate_t)); //TODO check error
	queue_init(&listaPuntatePrivata);

	argomentoGestorePuntate->listaPuntatePrivata = &listaPuntatePrivata;
	//argomentoGestorePuntate->clientFd = clientFd;

	while (1) {
		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
		datiGiocatore->budgetPrecedente = datiGiocatore->budgetAttuale;
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
		
		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		while (sessionePuntateCorrente.stato == 0) {
			pthread_cond_wait(&sessionePuntateCorrente.aperte, &sessionePuntateCorrente.mutex); //TODO check error

		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

		pthread_create(&tidGestorePuntateGiocatore, NULL, gestorePuntateGiocatore, (void *) argomentoGestorePuntate);

		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		while (sessionePuntateCorrente.stato == 1) {
			pthread_cond_wait(&sessionePuntateCorrente.chiuse, &sessionePuntateCorrente.mutex); //TODO check error

		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

		pthread_cancel(tidGestorePuntateGiocatore); //TODO check error


		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
		// collegare pacchetto di puntate alla lista del giocatore
		datiGiocatore->elencoPuntate.head = listaPuntatePrivata.head;
		sessioneGiocoCorrente.giocatoriChePuntano--;
		if(sessioneGiocoCorrente.giocatoriChePuntano == 0) {
			pthread_cond_signal(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate); //TODO check error
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

		queue_init(&listaPuntatePrivata);
		
		//* Aspettare che il croupier gestisca le puntate
		//* Quando segnalato, far partire la gestione dei messaggi tra client
	}
	free(argomentoGestorePuntate);
	pthread_exit(NULL);
}

void *gestorePuntateGiocatore(void *arg) {
	argomento_gestore_puntate_t *argomento = (argomento_gestore_puntate_t *) arg;
	size_t nbytes;
	ssize_t bytes_read;
	char stringaPuntata[10];
	puntata_t *singolaPuntata;

	bzero(stringaPuntata, sizeof (stringaPuntata));
	while (1) {
		/* riceve una stringa dal client del tipo "int tipo:int somma" dove:
		 * tipo == -1 significa puntata sui dispari
		 * tipo == -2 significa puntata sui pari
		 * tipo >= 0 rappresenta il numero puntato
		 * somma rappresenta la somma puntata
		 */
		nbytes = sizeof (stringaPuntata);
		bytes_read = read(argomento->clientFd, stringaPuntata, nbytes); //TODO check error
		
		//controlla se può scalare i soldi
		
		
		singolaPuntata = (puntata_t *) malloc(sizeof (singolaPuntata)); //TODO check error

		sscanf(stringaPuntata, "%d:%d", &singolaPuntata->tipoPuntata, &singolaPuntata->sommaPuntata);
		singolaPuntata->numeroPuntato = singolaPuntata->tipoPuntata;

		queue_put(argomento->listaPuntatePrivata, (node *) singolaPuntata);

		bzero(stringaPuntata, sizeof (stringaPuntata));
	}
	pthread_exit(NULL);
}