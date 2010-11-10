/*estrazione.c
 * =COMPILAZIONE=
 * gcc -Wall -l pthread -o server-roulette estrazione.c common_header.h common_header.c
 *
 * TODO inserire descrizione file
 */
#define DEBUG 1
#include "common_header.h"
#include "queue.h"
#include "control.h"

//TODO inserire descrizioni e nomi significativi per le variabili globali
pthread_mutex_t puntate_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t puntate_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t croupier_cond = PTHREAD_COND_INITIALIZER;

struct lista_puntate {
	data_control control;
	queue puntate;
} lp;

/* Quando estratto è -1 vuol dire che le puntate sono chiuse, quando è un
 * numero positivo le puntate sono aperte */
int estratto = -1;

//TODO inserire descrizione funzione

/**
 * FUNZIONE croupier
 *
 *============================================================================*/
void *croupier(void *arg) {
	struct timespec cond_time; //c'è solo cond_time in questa struct
	time_t now; //Conta i secondi da gennaio 1970 alle ore 00:00:00
	int status;
	int intervallo = (int) arg;
	struct request *puntata;

	//inizializzo il seme per la generazione di numeri random
	srand(time(NULL));

	while (1) {
		//Blocco il mutex per il croupier che fa l'estrazione
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel croupier");
		}
		//estrazione del numero 
		estratto = rand() % 37;

		printf("CROUPIER estratto=%d\n", estratto);

		now = time(NULL);
		cond_time.tv_sec = now + intervallo;
		cond_time.tv_nsec = 0;
		/* wake up players */
		status = pthread_cond_broadcast(&puntate_cond);
		if (status != 0) {
			err_abort(status, "Broadcast condition in croupier");
		}
		//Attende che la condizione sia true in un tempo specificato da cond_time
		while (estratto > 0) {
			status = pthread_cond_timedwait(&croupier_cond, &puntate_mutex, &cond_time); //TODO inserire gestione errori
			//Se status == ETIMEDOUT, significa che è scaduto il tempo senza la verifica della condizione
			if (status == ETIMEDOUT) {
				printf("CROUPIER tempo scaduto!!! chiudo le puntate\n");
				estratto = -1; //bets closed
				break;
			}
			if (status != 0) {
				err_abort(status, "Timedwait croupier");
			}
		}

		//gestione della puntata
		printf("CROUPIER Gestisco la puntata\n");
		//TODO funzione che gestisce le puntate ovvero controlla i vincitori
		while((puntata = get_request()) != NULL){
			printf("CROUPIER: nella lista delle puntate numero %d", puntata->number);
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
			pthread_cond_wait(&puntate_cond, &puntate_mutex); //TODO inserire gestione errori
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
		
/*
		printf("[T%d]?>", num);
		nbytes = sizeof (buf);
		bytes_read = read(STDIN_FILENO, buf, nbytes);
		puntato = atoi(buf);
*/
		puntato = rand() % 100;
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel player");
		}
		add_request(puntato);
		printf("GIOCATORE %d ha aggiunto %d\n", num, puntato);
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
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
	int status; /* raccoglie i valori restituiti
											dalle system call */
	pthread_t thread, croupier_tid;
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
	status = listen(sockfd, 5);
	if (status != 0) {
		err_abort(errno, "Error in listening to socket");
	}

	for (j = 0; j < 10; j++) {
		status = pthread_create(&thread, NULL, player, (void *) j);
		if (status != 0) {
			err_abort(status, "Creazione thread");
		}
	}

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

		status = pthread_create(&thread, NULL, player, (void *) client_info);
		if (status != 0) {
			err_abort(status, "Creazione thread");
		}
	}
	close(sockfd);
	return 0;
}
