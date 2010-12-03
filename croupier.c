#include "common_header.h"
#include "queue.h"

void *croupier(void *arg) {
	struct timespec intervalloGiocate;
	int intervalloInSecondi = 10; //questo viene passato dal main
	int status;
	int numeroEstratto;
	player_t *tempHead;
	
	while(1) {
		intervalloGiocate = calcola_intervallo(intervalloInSecondi);
		
		pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
		sessionePuntateCorrente.stato = 1;
		pthread_cond_broadcast(&sessionePuntateCorrente.aperte); //TODO check error 
		
		status = pthread_cond_timedwait(&sessionePuntateCorrente.attesaCroupier, &sessionePuntateCorrente.mutex, intervalloGiocate); //TODO check error
		if (status == ETIMEDOUT) {
			sessionePuntateCorrente.stato = 0;
			pthread_cond_broadcast(&sessionePuntateCorrente.chiuse); //TODO check error
		}
		pthread_mutex_unlock(&sessionePuntateCorrente.mutex); //TODO check error
		
		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
		sessioneGiocoCorrente.giocatoriChePuntano = sessioneGiocoCorrente.giocatoriConnessi;
		while(sessioneGiocoCorrente.giocatoriChePuntano > 0) {
			pthread_cond_wait(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate, &sessioneGiocoCorrente.mutex); //TODO check error
		}
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
		
		numeroEstratto = rand() % 37;
		//processare la lista delle puntate
		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
		
		tempHead = (player_t *)sessioneGiocoCorrente.elencoGiocatori.head;
		while(tempHead != NULL) {
			//scorri la lista delle puntate
			tempHead = (player_t *)tempHead->next;
		}
		
		
		
		
		
		//scorre la lista dei giocatori
		//di ogni giocatore scorre la lista delle puntate
		//se la lista finisce con tutte puntate vincenti, inserisci il giocatore nella lista dei vincitori e aumenta il numero dei vincitori
		//se la lista contiene una puntata perdente, inserisci il giocatore nella lista dei perdenti e aumenta il numero dei perdenti
		pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error
		
	}
}
