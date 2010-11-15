/* estrazione.c
 * =COMPILAZIONE=
 * gcc -Wall -l pthread -o server-roulette estrazione.c common_header.h
 *  common_header.c
 *
 * TODO inserire descrizione file
 */
#define DEBUG 1
#define CREATE_LOG 1

#include "common_header.h"

//#include "list_management.h"
//TODO inserire descrizioni e nomi significativi per le variabili globali
pthread_mutex_t puntate_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t puntate_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t croupier_cond = PTHREAD_COND_INITIALIZER;


/* Quando estratto è -1 vuol dire che le puntate sono chiuse, quando è un
 * numero positivo le puntate sono aperte */
int puntate_aperte = -1;

//TODO inserire descrizione funzione

/**
 * FUNZIONE croupier
 *
 *============================================================================*/
void *croupier(void *arg) {
	FILE *log_file;
	char *log_file_name = "croupier-log.txt";
	struct timespec cond_time; //c'è solo cond_time in questa struct
	int status;
	int intervallo = (int) arg;
	int estratto;
	player_t *player;



	log_file = fopen(log_file_name, "w");
	if (log_file == NULL) {
		fprintf(stderr, "Impossibile aprire il log file");
	}

	while (1) {
		//Blocco il mutex per il croupier che fa l'estrazione
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel croupier");
		}



#ifdef CREATE_LOG
		fprintf(log_file, "CROUPIER estratto=%d\n", estratto);
#endif

		cond_time = calcola_intervallo(intervallo);

		puntate_aperte = 1;
		/* wake up players */
		status = pthread_cond_broadcast(&puntate_cond);
		if (status != 0) {
			err_abort(status, "Broadcast condition in croupier");
		}

		while (puntate_aperte > 0) {
			status = pthread_cond_timedwait(&croupier_cond, &puntate_mutex,
				&cond_time);

			if (status == ETIMEDOUT) {
#ifdef CREATE_LOG
				fprintf(log_file, "CROUPIER tempo scaduto!!! chiudo le puntate\n");
#endif
				puntate_aperte = -1; //bets closed
				break;
			}
			if (status != 0) {
				err_abort(status, "Timedwait croupier");
			}
		}

		//gestione della puntata
		//sleep(2);
		//TODO inserire funzione che controlla i vincitori
		//estrazione del numero da 0 a 36
		estratto = rand() % 37;

		status = pthread_mutex_lock(&(players_list.control.mutex));
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel player");
		}
		printf("Ci sono %d giocatori per questa giocata\n", players_list.num_giocatori);
		printf("Ecco i loro nomi:\n");
		while ((player = (player_t *) queue_get(&(players_list.giocatori))) != NULL) {
			printf("%s\n", player->nickname);
		}



		status = pthread_mutex_unlock(&(players_list.control.mutex));
		if (status != 0) {
			err_abort(status, "Unlock sul mutex nel player");
		}

		gestisci_puntate(estratto);

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
	puntata_t *mybet;
	player_t *dati_player;
	//	size_t nbytes;
	//	ssize_t bytes_read;

	dati_player = malloc(sizeof (player_t));
	if (!dati_player) {
		printf("Errore malloc!\n");
		abort();
	}
	//TODO questi dati devono essere presi dal client, ovviamente
	dati_player->money = (rand() % MAX_BUDGET) + 1;
	snprintf(dati_player->nickname, sizeof (dati_player->nickname),
		"%s%d", "Giocatore", num_giocatore);

	status = pthread_mutex_lock(&(players_list.control.mutex));
	if (status != 0) {
		err_abort(status, "Lock sul mutex nel player");
	}
	players_list.num_giocatori++;
	queue_put(&(players_list.giocatori), (node *) dati_player);
	status = pthread_mutex_unlock(&(players_list.control.mutex));
	if (status != 0) {
		err_abort(status, "Unlock sul mutex nel player");
	}

#ifndef DEBUG
	/* Recupero le info dal client */
	client_t *client = (client_t *) arg;

	player = malloc(sizeof (player_t));

	/* leggo la porta di congratulazioni */
	nbytes = sizeof (in_port_t);
	bytes_read = read(client->clientfd, &(player->congrat_port), nbytes);
	if (bytes_read < 0) {
		err_abort(errno, "Lettura Porta Congratulazioni");
	}

	/* leggo i soldi */
	nbytes = sizeof (int)
		bytes_read = read(client->clientfd, &(player->money), nbytes);
	if (bytes_read < 0) {
		err_abort(errno, "Lettura somma giocatore");
	}

	/* leggo il nickname */
	//TODO mettere un valore costante al posto di 50
	bytes_read = read(client->clientfd, player->name, 50);
	if (bytes_read < 0) {
		err_abort(errno, "Lettura Nick Giocatore");
	}
	printf("\n===== DATI GIOCATORE =====\n");
	printf("Nickname: %s\n", player->name);
	printf("Soldi: %d\n", player->money);
	printf("Porta congratulazioni: %d\n", player->congrat_port);

	//TODO inserire le info nella lista dei giocatori
#endif

	status = pthread_mutex_lock(&puntate_mutex);
	if (status != 0) {
		err_abort(status, "Lock sul mutex nel player");
	}
	while (1) {
		/* in realtà la condizione (estratto < 0) va intesa come
		 * (puntate_aperte == 1) */
		while (puntate_aperte < 0) {
			printf("GIOCATORE %d CONDIZIONE FALSA\n", num_giocatore);
			status = pthread_cond_wait(&puntate_cond, &puntate_mutex);
			if (status != 0) {
				err_abort(status, "Wait nel player");
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
		//TODO leggere valori dal client
		//questi valori in realtà vengono presi dal client
		num_puntato_dal_giocatore = rand() % 37;
		tipo_puntata = (bet_t) (rand() % 3);
		somma_puntata = (rand() % 100) + 1;

		//lock del mutex per leggere money nella lista dei giocatori
		status = pthread_mutex_lock(&(players_list.control.mutex));
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel player");
		}
		if (somma_puntata <= dati_player->money) {
			status = pthread_mutex_unlock(&(players_list.control.mutex));
			if (status != 0) {
				err_abort(status, "Unlock sul mutex nel player");
			}
			sleep(1); //TODO rimuovere questa sleep
			status = pthread_mutex_lock(&puntate_mutex);
			if (status != 0) {
				err_abort(status, "Lock sul mutex nel player");
			}

			/* aggiunge un nodo alla lista delle puntate
			 * TODO modificare e integrare con funzioni di Antonio
			 */
			mybet = (puntata_t *) malloc(sizeof (puntata_t));
			if (!mybet) {
				err_abort(errno, "Errore malloc!");
			}
			mybet->numero = num_puntato_dal_giocatore;
			mybet->tipo = tipo_puntata;
			mybet->somma_puntata = somma_puntata;
			queue_put(&(lista_puntate.puntate), (node *) mybet);
			printf("GIOCATORE %d ha puntato il %d di tipo %d puntando %d€\n",
				num_giocatore, mybet->numero, mybet->tipo, mybet->somma_puntata);
			num_requests++;
		} else {
			//puntata non valida: somma troppo alta
			printf("GIOCATORE %d: somma troppo alta, ritenta\n", num_giocatore);
		}
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel player");
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

	//inizializzo il seme per la generazione di numeri random
	srand(time(NULL));
	//inizializza strutture dati
	queue_init(&(lista_puntate.puntate));
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
