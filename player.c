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
	/*
	 * Quando è già in atto una puntata, non si possono connettere nuovi giocatori
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
	
	datiGiocatore->datiConnessioneClient = (client_t *)arg;
	//TODO error check sulle read
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, &datiGiocatore->portaMessaggiCongratulazioni, sizeof (in_port_t));
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, &datiGiocatore->budgetAttuale, sizeof (int));
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, &nicknameLen, sizeof(size_t));
	bytesRead = read(datiGiocatore->datiConnessioneClient->clientFd, datiGiocatore->nickname, nicknameLen);
	
	datiGiocatore->portaMessaggiCongratulazioni = ntohs(datiGiocatore->portaMessaggiCongratulazioni);
	printf("====Dati Giocatore====\n");
	printf("Nickname: %s\n", datiGiocatore->nickname);
	printf("Budget Iniziale: %d\n", datiGiocatore->budgetAttuale);
	printf("Porta Congratulazioni: %d\n\n", datiGiocatore->portaMessaggiCongratulazioni);
	
	
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
		if (sessioneGiocoCorrente.giocatoriChePuntano == 0) {
			pthread_cond_signal(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate); //TODO check error
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

		queue_init(&listaPuntatePrivata);
		queue_init(argomentoGestorePuntate->listaPuntatePrivata);

		//* Aspettare che il croupier gestisca le puntate
		//* Quando segnalato, far partire la gestione dei messaggi tra client

		pthread_cond_wait(&analisiSessionePuntata.attesaMessaggi, &analisiSessionePuntata.mutex); //TODO check error

		//gestione messaggi tra client

		/*non si può inviare tramite write una struttura.
		  la scompattiamo e la inviamo al client una parte 
		  alla volta*/

		//write("numero perdenti");
		//write("numero vincitori");

		/*non possiamo inviare la lista dei vincitori
		  dobbiamo scompattare anche questa*/
		//Loop per ogni nodo della lista dei vincitori
		//write("indirizzo ip");
		//write("portamessaggi");

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
	
	
	printf("In attesa delle puntate\n");
	bzero(stringaPuntata, sizeof (stringaPuntata));
	while (1) {
		/* riceve una stringa dal client del tipo "int tipo:int somma" dove:
		 * tipo == -1 significa puntata sui dispari
		 * tipo == -2 significa puntata sui pari
		 * tipo >= 0 rappresenta il numero puntato
		 * somma rappresenta la somma puntata
		 */
		bytes_read = read(argomento->clientFd, &tipoPuntata, sizeof(int));
		bytes_read = read(argomento->clientFd, &sommaPuntata, sizeof(int));
		
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


//======================DA ELIMINARE===================================
	//TODO questi dati devono essere presi dal client, ovviamente
	//datiGiocatore->budgetAttuale = (rand() % MAX_BUDGET) + 1;
	//snprintf(datiGiocatore->nickname, sizeof (datiGiocatore->nickname),
	//	"%s%d", "Giocatore", num_giocatore);
	//======================DA ELIMINARE===================================
