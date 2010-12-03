#include "common_header.h"
#include "queue.h"

void *croupier(void *arg) {
	struct timespec intervalloGiocate;
	int intervalloInSecondi = 10; //questo viene passato dal main
	int status;
	int numeroEstratto;
	int contVincitori=0;
	int contPerdenti=0;
	player_t *tempHead;
	puntata_t *puntata;
	vincitore_t  *singoloVincitore;
	
	
	while(1) {
		/*
		 * Se non ci sono giocatori connessi, aspettane almeno uno
		 */
		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
		while(sessioneGiocoCorrente.giocatoriConnessi == 0) {
			pthread_cond_wait(sessioneGiocoCorrente.attesaAlmenoUnGiocatore, sessioneGiocoCorrente.mutex); //TODO check error
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
		
		intervalloGiocate = calcola_intervallo(intervalloInSecondi);
		/*
		 * Apri le puntate e risveglia tutti i giocatori in attesa di puntare
		 */
		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		sessionePuntateCorrente.stato = 1;
		pthread_cond_broadcast(&sessionePuntateCorrente.aperte); //TODO check error 
		status = pthread_cond_timedwait(&sessionePuntateCorrente.attesaCroupier, &sessionePuntateCorrente.mutex, intervalloGiocate); //TODO check error
		if (status == ETIMEDOUT) {
			/*
			 * Chiudi le puntate, allo scadere del tempo
			 */
			sessionePuntateCorrente.stato = 0;
			/*
			 * Memorizza il numero di giocatori che hanno partecipato all'ultima
			 * sessione di puntata e avvisa che le puntate sono chiuse
			 */
			pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
			sessioneGiocoCorrente.giocatoriChePuntano = sessioneGiocoCorrente.giocatoriConnessi;
			pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
			
			pthread_cond_broadcast(&sessionePuntateCorrente.chiuse); //TODO check error
		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error
		/*
		 * Aspetta che tutti i giocatori salvino i propri pacchetti di puntate
		 * nella lista comune
		 */
		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error		
		while(sessioneGiocoCorrente.giocatoriChePuntano > 0) {
			pthread_cond_wait(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate, &sessioneGiocoCorrente.mutex); //TODO check error
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
		
		numeroEstratto = rand() % 37;
		/*
		 * processare la lista delle puntate
		 */
		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
		tempHead = (player_t *)sessioneGiocoCorrente.elencoGiocatori.head;
		queue_init(&analisiSessionePuntata->elencoVincitori);
		
		while(tempHead != NULL) {
			tempHead->budgetPrecedente = tempHead->budgetAttuale;
			//scorri la lista delle puntate
			while(puntata = (puntata_t *)queue_get(&(tempHead->elencoPuntate)) != NULL) {
				/*
				 * Decrementa il budget della somma puntata. Il controllo della
				 * validità della puntata viene fatto nel client
				 */
				tempHead->budgetAttuale = tempHead->budgetAttuale - (puntata->sommaPuntata);
				//gestione della puntata in base al suo tipo.
				if((puntata->tipoPuntata >= 0) && (puntata->tipoPuntata <=36)) {
					gestisci_puntata_numero(numeroEstratto, puntata, tempHead);
				} else if(puntata->tipoPuntata == -1) {
					gestisci_puntata_pari(numeroEstratto, puntata, tempHead);
				} else if(puntata->tipoPuntata == -2) {
					gestisci_puntata_dispari(numeroEstratto, puntata, tempHead);
				}
				free(puntata);
			}
			 /*
			  * controlliamo quali giocatori sono vincitori e quali perdenti
			  */
			if(tempHead->budgetAttuale > tempHead->budgetPrecedente) {
				contVincitori++;
				singoloVincitore = (vincitore_t *)malloc(sizeof(vincitore_t));
				//TODO controllare se mettere solo in una sockaddr
				singoloVincitore->indirizzoIp.sin_addr = tempHead->datiConnessioneClient->client_data.sin_addr;
				singoloVincitore->indirizzoIp.sin_port = tempHead->datiConnessioneClient->client_data.sin_port;
				singoloVincitore->portaMessaggiCongratulazioni = tempHead->portaMessaggiCongratulazioni;
				
				pthread_mutex_lock(&analisiSessionePuntata.mutex); //TODO check error
				queue_put(&analisiSessionePuntata->elencoVincitori,(node *)singoloVincitore);
				pthread_mutex_unlock(&analisiSessionePuntata.mutex); //TODO check error
			} else {
				contPerdenti++;	
			}
			
			tempHead = (player_t *)tempHead->next;
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
		
		//memorizziamo il numero di vincitori e di perdenti
		pthread_mutex_lock(&analisiSessionePuntata.mutex); //TODO check error
		analisiSessionePuntata->numeroVincitori = contVincitori;
		analisiSessionePuntata->numeroPerdenti = contPerdenti;
		pthread_cond_broadcast(&analisiSessionePuntata.attesaMessaggi); //TODO check error
		pthread_mutex_unlock(&analisiSessionePuntata.mutex); //TODO check error
		
		/*la lista delle puntate è stata processata completamente.
		  la lista dei vincitori è stata riempita.
		  ora il croupier può segnalare che ha concluso la gestione
		  delle puntate*/
		
		contPerdenti=contVincitori=0;
		
		  		
		
		
	}
}
