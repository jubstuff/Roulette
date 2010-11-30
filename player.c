#include "common_header.h"

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
	queue *listaPuntatePrivata;

	queue_init(listaPuntatePrivata);

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
	while (1) {
		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		while (sessionePuntateCorrente.stato == 0) {
			pthread_cond_wait(&sessionePuntateCorrente.aperte, &sessionePuntateCorrente.mutex); //TODO check error

		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

		pthread_create(&tidGestorePuntateGiocatore, NULL, gestorePuntateGiocatore, NULL);

		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		while (sessionePuntateCorrente.stato == 1) {
			pthread_cond_wait(&sessionePuntateCorrente.chiuse, &sessionePuntateCorrente.mutex); //TODO check error

		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

		pthread_cancel(tidGestorePuntateGiocatore); //TODO check error

		/*
		 * Prendere la lista privata e concatenarla a quella comune
		 * reinizializza il puntatore della lista privata a NULL
		 * Aspettare che il croupier gestisca le puntate
		 * Quando segnalato, far partire la gestione dei messaggi tra client
		 */
	}
	pthread_exit(NULL);
}


void *gestorePuntateGiocatore(void *arg) {
	int num;
	
	while (1) {
		nodo_puntata = (node_t *) malloc(sizeof (node_t));
		num = rand() % 37;
		nodo_puntata->data = num;
		printf("[%08x]Inserisco una puntata[%d]\n", par->padre, num);
		queue_put(par->lista_privata, (node *) nodo_puntata);
		sleep(1); //per limitare il numero di inserimenti nella lista privata
	}
	
	pthread_exit(NULL);
}