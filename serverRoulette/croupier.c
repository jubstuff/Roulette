#include "common_header.h"
#include "queue.h"

void *croupier(void *arg) {
    struct timespec intervalloGiocate;
    int intervalloInSecondi = (int) arg; //questo viene passato dal main
    int status;
    int numeroEstratto;
    int contVincitori = 0;
    int contPerdenti = 0;
    player_t *tempPlayer = NULL;
    puntata_t *puntata = NULL;
    vincitore_t *singoloVincitore = NULL;
    printf("Preparo il tavolo da gioco.\n");

    while (1) {
        /*
         * Se non ci sono giocatori connessi, aspettane almeno uno
         * ASK ma anche se non ci sono puntate?
         */
        pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
        while (sessioneGiocoCorrente.giocatoriConnessi == 0) {
            pthread_cond_wait(&sessioneGiocoCorrente.attesaAlmenoUnGiocatore, &sessioneGiocoCorrente.mutex); //TODO check error
        }
        pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

        intervalloGiocate = calcola_intervallo(intervalloInSecondi);
        /*
         * Apri le puntate e risveglia tutti i giocatori in attesa di puntare
         */
        printf("[Croupier] Apro le puntate\n");
        pthread_mutex_lock(&sessionePuntateCorrente.mutex); //TODO check error
        sessionePuntateCorrente.stato = 1;
        pthread_cond_broadcast(&sessionePuntateCorrente.aperte); //TODO check error
        status = pthread_cond_timedwait(&sessionePuntateCorrente.attesaCroupier, &sessionePuntateCorrente.mutex, &intervalloGiocate); //TODO check error
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
        printf("[Croupier] Aspetto che i giocatori salvino le puntate\n");
        pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
        while (sessioneGiocoCorrente.giocatoriChePuntano > 0) {
            pthread_cond_wait(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate, &sessioneGiocoCorrente.mutex); //TODO check error
        }
        pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

        //numeroEstratto = rand() % 37;
		//TODO rimettere generazione casuale
        //Just:rendo il numero statico, per provare vittorie e perdite
        numeroEstratto = 33;
        printf("[Croupier] È stato estratto il numero %d\n", numeroEstratto);
        /*
         * processare la lista delle puntate
         */
        pthread_mutex_lock(&analisiSessionePuntata.mutex); //TODO check error
        queue_init(&(analisiSessionePuntata.elencoVincitori));
		analisiSessionePuntata.stato = 0;
        pthread_mutex_unlock(&analisiSessionePuntata.mutex); //TODO check error

		pthread_mutex_lock(&sessioneGiocoCorrente.mutex); //TODO check error
        tempPlayer = (player_t *) sessioneGiocoCorrente.elencoGiocatori.head;
        while (tempPlayer != NULL) {
            tempPlayer->budgetPrecedente = tempPlayer->budgetAttuale;
            //scorri la lista delle puntate
            while ((puntata = (puntata_t *) queue_get(&tempPlayer->elencoPuntate)) != NULL) {
                /*
                 * Decrementa il budget della somma puntata. Il controllo della
                 * validità della puntata viene fatto nel client
                 */
                tempPlayer->budgetAttuale = tempPlayer->budgetAttuale - (puntata->sommaPuntata);
                //gestione della puntata in base al suo tipo.
                if ((puntata->tipoPuntata >= 0) && (puntata->tipoPuntata <= 36)) {
                    gestisci_puntata_numero(numeroEstratto, puntata, tempPlayer);
                } else if (puntata->tipoPuntata == -1) {
                    //gestisci_puntata_dispari(numeroEstratto, puntata, tempPlayer);
					aumenta_budget(2, puntata, tempPlayer);
                } else if (puntata->tipoPuntata == -2) {
                    //gestisci_puntata_pari(numeroEstratto, puntata, tempPlayer);
					aumenta_budget(2, puntata, tempPlayer);  //TODO lasciare aumenta_budget o tornare a gestisci_puntate?
                }
                free(puntata);
                printf("[Croupier] Budget Attuale di %s: %d\n", tempPlayer->nickname, tempPlayer->budgetAttuale);
            }
            /*
             * controlliamo quali giocatori sono vincitori e quali perdenti
             */
            if (tempPlayer->budgetAttuale > tempPlayer->budgetPrecedente) {
                contVincitori++;
				tempPlayer->vincitore = 1;
                singoloVincitore = (vincitore_t *) malloc(sizeof (vincitore_t));
                //TODO controllare se mettere solo in una sockaddr
                singoloVincitore->indirizzoIp.sin_addr = tempPlayer->datiConnessioneClient->clientData.sin_addr;
                singoloVincitore->indirizzoIp.sin_port = tempPlayer->datiConnessioneClient->clientData.sin_port;
                singoloVincitore->portaMessaggiCongratulazioni = tempPlayer->portaMessaggiCongratulazioni;

                pthread_mutex_lock(&analisiSessionePuntata.mutex); //TODO check error
                queue_put(&analisiSessionePuntata.elencoVincitori, (node *) singoloVincitore);
                pthread_mutex_unlock(&analisiSessionePuntata.mutex); //TODO check error
            } else if(tempPlayer->budgetAttuale == 0) {
				//TODO
				//stacca il nodo dalla lista
				//manda un messaggio
				//disconnetti
				//pulisci la memoria
				contPerdenti++;
			}
			else {
                contPerdenti++;
            }

            tempPlayer = (player_t *) tempPlayer->next;
        }
        pthread_mutex_unlock(&sessioneGiocoCorrente.mutex); //TODO check error

        printf("Ci sono stati %d vincitori e %d perdenti.\n", contVincitori, contPerdenti);
        //memorizziamo il numero di vincitori e di perdenti
        pthread_mutex_lock(&analisiSessionePuntata.mutex); //TODO check error
        analisiSessionePuntata.numeroVincitori = contVincitori;
        analisiSessionePuntata.numeroPerdenti = contPerdenti;
		analisiSessionePuntata.stato = 1;
        pthread_cond_broadcast(&analisiSessionePuntata.attesaMessaggi); //TODO check error
        pthread_mutex_unlock(&analisiSessionePuntata.mutex); //TODO check error

        /*la lista delle puntate è stata processata completamente.
          la lista dei vincitori è stata riempita.
          ora il croupier può segnalare che ha concluso la gestione
          delle puntate*/

        contPerdenti = contVincitori = 0;
    }
}
