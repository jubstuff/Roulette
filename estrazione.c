/* estrazione.c
 * =COMPILAZIONE=
 * gcc -Wall -l pthread -o server-roulette estrazione.c common_header.h
 *  common_header.c
 *
 * TODO inserire descrizione file
 */
#define DEBUG 1
//#define CREATE_LOG 1

#include "common_header.h"

//#include "list_management.h"
//TODO inserire descrizioni e nomi significativi per le variabili globali
pthread_mutex_t puntate_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t puntate_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t croupier_cond = PTHREAD_COND_INITIALIZER;




//TODO inserire descrizione funzione

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
		//Blocco il mutex per il croupier che fa l'estrazione
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel croupier");
		}
		cond_time = calcola_intervallo(intervallo);
		//apre le puntate
		lista_puntate.stato_puntate = 1;
		/* wake up players */
		status = pthread_cond_broadcast(&puntate_cond);
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

/**
 * FUNZIONE player
 *
 *============================================================================*/
void *player(void *arg) {
	int num_giocatore = (int) arg;
	int num_puntato_dal_giocatore = 0;
	bet_t tipo_puntata;
	int status;
	int somma_puntata;
	puntata_t *mybet = NULL;
	player_t *dati_player = NULL;

	//alloca un nuovo nodo per la lista dei giocatori
	dati_player = (player_t *) malloc(sizeof (player_t));
	if (!dati_player) {
		printf("Errore malloc!\n");
		abort(); //FIXME che fare qui?
	}

	//======================DA ELIMINARE===================================
	//TODO questi dati devono essere presi dal client, ovviamente
	dati_player->money = (rand() % MAX_BUDGET) + 1;
	snprintf(dati_player->nickname, sizeof (dati_player->nickname),
			"%s%d", "Giocatore", num_giocatore);
	//======================DA ELIMINARE===================================


	status = pthread_mutex_lock(&(players_list.control.mutex));
	if (status != 0) {
		err_abort(status, "Lock all'inserimento del nodo giocatore");
	}
	//aggiunge il nodo alla lista dei giocatori
	players_list.num_giocatori++;
	queue_init(&(dati_player->lista_puntate_personale.puntate));
	queue_put(&(players_list.giocatori), (node *) dati_player);
	status = pthread_mutex_unlock(&(players_list.control.mutex));
	if (status != 0) {
		err_abort(status, "Unlock all'inserimento del nodo giocatore");
	}

	status = pthread_mutex_lock(&puntate_mutex);
	if (status != 0) {
		err_abort(status, "Lock sul mutex nel player");
	}
	while (1) {
		while (lista_puntate.stato_puntate == -1) { //se le puntate sono chiuse prima dell'estrazione, aspetta
			printf("GIOCATORE %d: TROVATO PUNTATE CHIUSE\n", num_giocatore);
			status = pthread_cond_wait(&puntate_cond, &puntate_mutex);
			if (status != 0) {
				err_abort(status, "Wait per l'apertura delle puntate");
			}
		}

		//here player can bet
		/*
		 * unlock mutex
		 * read bet on socket
		 * insert bet in list
		 * lock mutex
		 * */
		status = pthread_mutex_unlock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Unlock sul mutex nel player");
		}

		//======================DA ELIMINARE===================================
		//TODO questi valori in realtà vengono presi dal client
		num_puntato_dal_giocatore = rand() % 37;
		tipo_puntata = (bet_t) (rand() % 3);
		somma_puntata = (rand() % 100) + 1;
		//======================DA ELIMINARE===================================

		if (somma_puntata <= dati_player->money) {
			//puntata valida
			dati_player->money -= somma_puntata;


			mybet = inizializza_nodo_puntata(num_puntato_dal_giocatore,
											tipo_puntata, somma_puntata);
			status = pthread_mutex_lock(&puntate_mutex);
			if (status != 0) {
				err_abort(status, "Lock sul mutex nel player");
			}
			// aggiunge un nodo alla lista delle puntate
			queue_put(&(dati_player->lista_puntate_personale.puntate),
					(node *) mybet);
			printf("GIOCATORE %d ha puntato il %d di tipo %d puntando %d€\n",
				num_giocatore, mybet->numero, mybet->tipo, mybet->somma_puntata);
		} else {
			//puntata non valida: somma troppo alta
			printf("GIOCATORE %d: somma troppo alta, ritenta\n", num_giocatore);
		}
		sleep(1); //TODO rimuovere questa sleep

		while (lista_puntate.stato_puntate == 0) {
			printf("Il croupier sta processando la puntata, aspetto...\n");
			pthread_cond_wait(&croupier_cond, &puntate_mutex);
		}
	}
	pthread_exit(NULL);
}

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
	 * 1  => significa che le puntate sono aperte
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

#ifdef DEBUG
	printf("La giocata durerà %d secondi\n", game_interval);
#endif
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
