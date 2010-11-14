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
#include "list_management.h"
//TODO inserire descrizioni e nomi significativi per le variabili globali
pthread_mutex_t puntate_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t puntate_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t croupier_cond = PTHREAD_COND_INITIALIZER;


/* Quando estratto è -1 vuol dire che le puntate sono chiuse, quando è un
 * numero positivo le puntate sono aperte */
int estratto = -1;

//TODO inserire descrizione funzione

/**
 * FUNZIONE croupier
 *
 *============================================================================*/
void *croupier(void *arg) {
	FILE *log_file;
	char *log_file_name = "croupier-log.txt";

	struct timespec cond_time; //c'è solo cond_time in questa struct
	time_t now; //Conta i secondi da gennaio 1970 alle ore 00:00:00
	int status;
	int intervallo = (int) arg;
	pnode *puntata;

	//inizializzo il seme per la generazione di numeri random
	srand(time(NULL));

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

		//estrazione del numero da 1 a 36
		estratto = rand() % 37;

#ifdef CREATE_LOG
		fprintf(log_file, "CROUPIER estratto=%d\n", estratto);
#endif

		now = time(NULL);
		cond_time.tv_sec = now + intervallo;
		cond_time.tv_nsec = 0;

		/* wake up players */
		status = pthread_cond_broadcast(&puntate_cond);
		if (status != 0) {
			err_abort(status, "Broadcast condition in croupier");
		}

		while (estratto > 0) {
			status = pthread_cond_timedwait(&croupier_cond, &puntate_mutex,
				&cond_time);

			//Se status == ETIMEDOUT, significa che è scaduto il tempo senza la
			//verifica della condizione
			if (status == ETIMEDOUT) {

#ifdef CREATE_LOG
				fprintf(log_file, "CROUPIER tempo scaduto!!! chiudo le puntate\n");
#endif
				estratto = -1; //bets closed
				break;
			}

			if (status != 0) {
				err_abort(status, "Timedwait croupier");
			}
		}

		//gestione della puntata
#ifdef CREATE_LOG
		fprintf(log_file, "CROUPIER Gestisco la puntata. numrichieste = %d\n",
			num_requests);
#endif
		sleep(2);
		//TODO inserire funzione che controlla i vincitori
		while ((puntata = (pnode *) queue_get(&bl.puntate)) != NULL) {
#ifdef CREATE_LOG
			fprintf(log_file, "CROUPIER: nella lista delle puntate numero %d\n",
				puntata->puntata);
#endif
			num_requests--;
		}
		status = pthread_mutex_unlock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Unlock sul mutex nel player");
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
	int num = (int) arg;
	int puntato = 0;
	int status;
	pnode *mybet;
	player_t player;
	size_t nbytes;
	ssize_t bytes_read;



	FILE *log_file;
	char *log_file_name = "player-log.txt";


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

	//Il player prende il possesso del mutex
	status = pthread_mutex_lock(&puntate_mutex);
	if (status != 0) {
		err_abort(status, "Lock sul mutex nel player");
	}
	while (1) {
		/* in realtà la condizione (estratto < 0) va intesa come
		 * (puntate_aperte == 1) */
		while (estratto < 0) {/* if bets are opened */
			printf("GIOCATORE %d CONDIZIONE FALSA\n", num);
			status = pthread_cond_wait(&puntate_cond, &puntate_mutex);
			if (status != 0) {
				err_abort(status, "Wait nel player");
			}
		}

		//here player can bet
		/*
		 * read bet on socket
		 * unlock mutex
		 * insert bet in list
		 * lock mutex
		 * */
		status = pthread_mutex_unlock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Unlock sul mutex nel player");
		}
		//questo valore in realtà viene preso dal client
		puntato = rand() % 37;
		sleep(1);
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel player");
		}

		/* aggiunge un nodo alla lista delle puntate
		 * TODO modificare e integrare con funzioni di Antonio
		 */
		mybet = malloc(sizeof (pnode));
		if (!mybet) {
			err_abort(errno, "Errore malloc!" );
		}
		mybet->puntata = puntato;
		queue_put(&bl.puntate, (node *) mybet);
		num_requests++;
		printf("GIOCATORE %d ha aggiunto %d\n", num, puntato);
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
	int sockfd, clientfd; /* socket e client descriptor */
	short int server_port; /* porta del server */
	int game_interval; /* durata possibilità puntate */
	struct sockaddr_in self, client_addr; /* info del server e client */
	socklen_t client_len = sizeof (client_addr);
	int status; /* raccoglie i valori restituiti dalle system call */
	pthread_t player_tid, croupier_tid;
	client_t *client_info;

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
