#include "common_header.h"
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

	while (1) {
		status = pthread_mutex_lock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Lock sul mutex nel player");
		}
		while (lista_puntate.stato_puntate == -1) {
			//se le puntate sono chiuse prima dell'estrazione, aspetta
			status = pthread_cond_wait(&puntate_aperte, &puntate_mutex);
			if (status != 0) {
				err_abort(status, "Wait per l'apertura delle puntate");
			}
		}
		status = pthread_mutex_unlock(&puntate_mutex);
		if (status != 0) {
			err_abort(status, "Unlock sul mutex nel player");
		}
		//pthread_create(&gestore_puntate, NULL, gestore_func, (void *) (&par));
		

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


void *gestore_puntate(void *arg) {
	/*parametri_servizio_t *par = (parametri_servizio_t *) arg;
	node_t *nodo_puntata;
	int num;


	//printf("sono il thread figlio\n");
	pthread_cleanup_push(cleanup, NULL);
	while (1) {
		nodo_puntata = (node_t *) malloc(sizeof (node_t));
		num = rand() % 37;
		nodo_puntata->data = num;
		printf("[%08x]Inserisco una puntata[%d]\n", par->padre, num);
		queue_put(par->lista_privata, (node *) nodo_puntata);
		sleep(1); //per limitare il numero di inserimenti nella lista privata
	}
	pthread_cleanup_pop(0);
	pthread_exit(NULL);*/
}