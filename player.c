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
	/*
	 * Quando è già in atto una puntata, non si possono connettere nuovi giocatori
	 */
	pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
	while(sessionePuntateCorrente.stato == 1) {
		pthread_cond_wait(&sessionePuntateCorrente.chiuse, &sessionePuntateCorrente.mutex );
	}
	pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

	/*
	 * Recupera dati dal client e inserisci un nuovo giocatore nella lista
	 */
	datiGiocatore = (player_t *) malloc(sizeof (player_t)); //TODO check error
	//======================DA ELIMINARE===================================
	//TODO questi dati devono essere presi dal client, ovviamente
	datiGiocatore->budgetAttuale = (rand() % MAX_BUDGET) + 1;
	snprintf(datiGiocatore->nickname, sizeof (datiGiocatore->nickname),
			"%s%d", "Giocatore", num_giocatore);
	//======================DA ELIMINARE===================================

	pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
	queue_put(&sessioneGiocoCorrente.elencoGiocatori, (node *)datiGiocatore);
	sessioneGiocoCorrente.giocatoriConnessi++;
	/*
	 * Se questo thread è il primo giocatore connesso, avvisa il croupier che 
	 * può iniziare la giocata
	 */
	if( sessioneGiocoCorrente.giocatoriConnessi == 1) {
		pthread_cond_signal(sessioneGiocoCorrente.attesaAlmenoUnGiocatore); //TODO check error
	}
	pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

	/*
	 * Prepara l'argomento da passare al thread gestore delle puntate
	 */
	argomentoGestorePuntate = (argomento_gestore_puntate_t *) malloc(sizeof (argomento_gestore_puntate_t)); //TODO check error
	queue_init(&listaPuntatePrivata);
	argomentoGestorePuntate->listaPuntatePrivata = &listaPuntatePrivata;
	//argomentoGestorePuntate->clientFd = clientFd; //FIXME il thread deve ricevere il socket descriptor dal main

	while (1) {
		/*
		 * Aspetta che il croupier apra le puntate
		 */
		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		while (sessionePuntateCorrente.stato == 0) {
			pthread_cond_wait(&sessionePuntateCorrente.aperte, &sessionePuntateCorrente.mutex); //TODO check error
		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error

		pthread_create(&tidGestorePuntateGiocatore, NULL, gestorePuntateGiocatore, (void *) argomentoGestorePuntate);
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
		datiGiocatore->elencoPuntate.head = listaPuntatePrivata.head;
		sessioneGiocoCorrente.giocatoriChePuntano--;
		/*
		 * Se tutti i giocatori hanno collegato il proprio pacchetto di puntate
		 * risveglia il croupier
		 */
		if(sessioneGiocoCorrente.giocatoriChePuntano == 0) {
			pthread_cond_signal(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate); //TODO check error
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
		
		queue_init(&listaPuntatePrivata);
		
		//* Aspettare che il croupier gestisca le puntate
		//* Quando segnalato, far partire la gestione dei messaggi tra client
		
		pthread_cond_wait(&analisiSessionePuntata.attesaMessaggi, &analisiSessionePuntata.mutex); //TODO check error
		
		//gestione messaggi tra client
		
		/*non si può inviare tramite write una struttura.
		  la scompattiamo e la inviamo al client una parte 
		  alla volta*/
		
		write("numero perdenti");
		write("numero vincitori");
		
		/*non possiamo inviare la lista dei vincitori
		  dobbiamo scompattare anche questa*/
		//Loop per ogni nodo della lista dei vincitori
		write("indirizzo ip");
		write("portamessaggi");
		
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
		
		
		singolaPuntata = (puntata_t *) malloc(sizeof (singolaPuntata)); //TODO check error

		sscanf(stringaPuntata, "%d:%d", &singolaPuntata->tipoPuntata, &singolaPuntata->sommaPuntata);
		singolaPuntata->numeroPuntato = singolaPuntata->tipoPuntata;

		queue_put(argomento->listaPuntatePrivata, (node *) singolaPuntata);

		bzero(stringaPuntata, sizeof (stringaPuntata));
	}
	pthread_exit(NULL);
}