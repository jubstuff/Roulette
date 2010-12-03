/* estrazione.c
 * =COMPILAZIONE=
 * gcc -Wall -l pthread -o server-roulette estrazione.c common_header.h
 * common_header.c
 *
 * TODO inserire descrizione file
 */
#define DEBUG 1
//#define CREATE_LOG 1

#include "common_header.h"

//#include "list_management.h"
//TODO inserire descrizioni e nomi significativi per le variabili globali
pthread_mutex_t puntate_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t puntate_aperte = PTHREAD_COND_INITIALIZER;
pthread_cond_t croupier_cond = PTHREAD_COND_INITIALIZER;




// TODO inserire descrizione funzione

/**
 * FUNZIONE croupier
 *
 *============================================================================*/
void *croupier(void *arg) {
	struct timespec cond_time;
	int status;
	int intervallo = (int) arg;
	int estratto;

	while (1) {

		cond_time = calcola_intervallo(intervallo);
		//Blocco il mutex per il croupier che fa l'estrazione
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel croupier");
		}
		//apre le puntate
		lista_puntate.stato_puntate = 1;
		/* wake up players */
		status = pthread_cond_broadcast(&puntate_aperte);
		if (status != 0) {
			err_abort(status, "Broadcast condition in croupier");
		}

		while (lista_puntate.stato_puntate == 1) {
			status = pthread_cond_timedwait(&croupier_cond, &puntate_mutex,
											&cond_time);

			if (status == ETIMEDOUT) {
				lista_puntate.stato_puntate = -1; //chiude le puntate
				break;
			}
			if (status != 0) {
				err_abort(status, "Timedwait croupier");
			}
		}

		//estrazione del numero da 0 a 36
		estratto = rand() % 37;

		//gestione della puntata
		status = pthread_mutex_lock(&(players_list.control.mutex));
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel croupier");
		}
		printf("Ci sono %d giocatori per questa giocata\n",
			players_list.num_giocatori);

		gestisci_puntate(estratto);

		status = pthread_mutex_unlock(&(players_list.control.mutex));
		if (status != 0) {
			err_abort(status, "Unlock sul mutex nel croupier");
		}
		status = pthread_mutex_unlock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Unlock sul mutex nel croupier");
		}
	}
	pthread_exit(NULL);
}
//TODO inserire descrizione funzione




int main(int argc, char **argv) {
	//TODO impacchettare le funzionalità in funzioni
	//TODO fare il join dei thread al termine

	/* controllo numero di argomenti */
	if (argc != 3) {
		printf("Utilizzo: %s <numero porta> <intervallo secondi>\n", argv[0]);
		exit(1);
	} //TODO controllo errori più robusto

	int j = 0;
	int sockfd;
	//clientfd; /* socket e client descriptor */
	short int server_port; /* porta del server */
	int game_interval; /* durata possibilità puntate */
	struct sockaddr_in self; // client_addr; /* info del server e client */
	//socklen_t client_len = sizeof (client_addr);
	int status; /* raccoglie i valori restituiti dalle system call */
	pthread_t player_tid, croupier_tid;
	//client_t *client_info;


	/*
	 * STATO PUNTATE
	 *
	 * -1 => significa che le puntate sono chiuse
	 * 1 => significa che le puntate sono aperte
	 *
	 * */
	lista_puntate.stato_puntate = -1;
	//TODO mettere queste inizializzazioni in una funzione
	//inizializzo il seme per la generazione di numeri random
	srand(time(NULL));
	//inizializza strutture dati
	//queue_init(&(lista_puntate.puntate));
	queue_init(&(players_list.giocatori));
	control_init(&(players_list.control));
	control_activate(&(players_list.control));
	/* converto i parametri passati a interi */
	server_port = atoi(argv[1]);
	game_interval = atoi(argv[2]);


	printf("La giocata durerà %d secondi\n", game_interval);

	/* creo il thread CROUPIER */
	status = pthread_create(&croupier_tid, NULL, croupier, (void *) game_interval);
	if (status != 0) {
		err_abort(status, "Creazione del thread croupier");
	}

	sockfd = open_socket(self, server_port);
#ifdef DEBUG
	/* Crea 10 player thread */
	for (j = 0; j < 10; j++) {
		status = pthread_create(&player_tid, NULL, player, (void *) j);
		if (status != 0) {
			err_abort(status, "Creazione thread");
		}
	}
#endif
#ifndef DEBUG
	while (1) {
		/* accetta connessioni dai client */
		clientfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
		if (clientfd < 0) {
			err_abort(errno, "Error accepting connection");
		}
		/* inserisco informazioni sul client da inviare al thread player */
		client_info = NULL;
		client_info = malloc(sizeof (client_t));
		client_info->client_data = client_addr;
		client_info->clientfd = clientfd;

		status = pthread_create(&player_tid, NULL, player, (void *) client_info);
		if (status != 0) {
			err_abort(status, "Creazione thread");
		}
	}
#endif
	close(sockfd);
	pthread_exit(NULL);
}