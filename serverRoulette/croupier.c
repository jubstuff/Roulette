/**
 * @file   croupier.c
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Procedura per il thread croupier
 *
 */
#include "../common/common_header.h"
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
    int partecipantiAllaPuntata = 0;
    int nonPartecipantiAllaPuntata = 0;
    printf("Preparo il tavolo da gioco.\n");

    while (1) {
        /*
         * Se non ci sono giocatori connessi, aspettane almeno numMinimo
         * ASK ma anche se non ci sono puntate?
         */
        Pthread_mutex_lock(&sessioneGiocoCorrente.mutex);
        while (sessioneGiocoCorrente.giocatoriConnessi < numeroMinimoGiocatori) {
            Pthread_cond_wait(&sessioneGiocoCorrente.attesaAlmenoUnGiocatore, &sessioneGiocoCorrente.mutex);
        }
        
        Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);

        intervalloGiocate = calcola_intervallo(intervalloInSecondi);
        /*
         * Apri le puntate e risveglia tutti i giocatori in attesa di puntare
         */
        printf("\n\n[Croupier] Apro le puntate\n");

        
        Pthread_mutex_lock(&sessionePuntateCorrente.mutex);
        sessionePuntateCorrente.stato = 1;
        Pthread_cond_broadcast(&sessionePuntateCorrente.aperte);
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
            Pthread_mutex_lock(&sessioneGiocoCorrente.mutex);
            //TODO attenzione qui: il numero dei giocatori che puntano è uguale
            //al numero dei giocatori connessi **all'inizio** della puntata, non
            //in generale!!!
            sessioneGiocoCorrente.giocatoriChePuntano = sessioneGiocoCorrente.giocatoriConnessi;
            //questo è per il controllo interno del croupier sui giocatori
            //che si sono connessi dopo l'inizio della puntata corrente
            partecipantiAllaPuntata = sessioneGiocoCorrente.giocatoriConnessi;
            Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);
            
            //=============AGGIUNTO PER PROVA===================================
            Pthread_mutex_lock(&analisiSessionePuntata.mutex);
            analisiSessionePuntata.stato = 0;
            Pthread_mutex_unlock(&analisiSessionePuntata.mutex);
            //=============FINE AGGIUNTO PER PROVA==============================

            Pthread_cond_broadcast(&sessionePuntateCorrente.chiuse);
        }
        Pthread_mutex_unlock(&sessionePuntateCorrente.mutex);
        /*
         * Aspetta che tutti i giocatori salvino i propri pacchetti di puntate
         * nella lista comune
         */
        printf("\n[Croupier] Aspetto che i giocatori salvino le puntate\n");
        Pthread_mutex_lock(&sessioneGiocoCorrente.mutex);
        while (sessioneGiocoCorrente.giocatoriChePuntano > 0) {
            Pthread_cond_wait(&sessioneGiocoCorrente.attesaRiempimentoListaPuntate, &sessioneGiocoCorrente.mutex);
        }
        Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);

        


        //numeroEstratto = rand() % 37;
        //TODO rimettere generazione casuale
        //Just:rendo il numero statico, per provare vittorie e perdite
        numeroEstratto = 33;
        printf("[Croupier] È stato estratto il numero %d\n", numeroEstratto);
        //qui stava analisiSessione.stato = 0
        /*
         * processare la lista delle puntate
         */
        Pthread_mutex_lock(&analisiSessionePuntata.mutex);
        queue_init(&(analisiSessionePuntata.elencoVincitori));
//        analisiSessionePuntata.stato = 0;
        analisiSessionePuntata.numeroVincitori = 0;
        analisiSessionePuntata.numeroPerdenti = 0;
        Pthread_mutex_unlock(&analisiSessionePuntata.mutex);

        Pthread_mutex_lock(&sessioneGiocoCorrente.mutex);
        tempPlayer = (player_t *) sessioneGiocoCorrente.elencoGiocatori.head;
        while (tempPlayer != NULL) {
            tempPlayer->vincitore = 0;
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
                    gestisci_puntata_dispari(numeroEstratto, puntata, tempPlayer);
                    
                } else if (puntata->tipoPuntata == -2) {
                    gestisci_puntata_pari(numeroEstratto, puntata, tempPlayer);
                    
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

                Pthread_mutex_lock(&analisiSessionePuntata.mutex);
                queue_put(&analisiSessionePuntata.elencoVincitori, (node *) singoloVincitore);
                Pthread_mutex_unlock(&analisiSessionePuntata.mutex);
            } else if (tempPlayer->budgetAttuale == 0) {
                //TODO
                //stacca il nodo dalla lista
                //manda un messaggio
                //disconnetti
                //pulisci la memoria
                tempPlayer->vincitore = 0;
                contPerdenti++;
            } else {
            	tempPlayer->vincitore = 0;
                contPerdenti++;
            }

            tempPlayer = (player_t *) tempPlayer->next;
        }
        
        if(sessioneGiocoCorrente.giocatoriConnessi > partecipantiAllaPuntata) {
            nonPartecipantiAllaPuntata = sessioneGiocoCorrente.giocatoriConnessi - partecipantiAllaPuntata;
            contPerdenti = contPerdenti - (nonPartecipantiAllaPuntata);
        }
        Pthread_mutex_unlock(&sessioneGiocoCorrente.mutex);

        printf("Ci sono stati %d vincitori e %d perdenti.\n", contVincitori, contPerdenti);
        //memorizziamo il numero di vincitori e di perdenti
        Pthread_mutex_lock(&analisiSessionePuntata.mutex);
        
	//stampa l'elenco dei vincitori
	printf("\n[Croupier] Stampo l'elenco dei vincitori.\n");
        vincitore_t *temp = (vincitore_t *)analisiSessionePuntata.elencoVincitori.head;
        while(temp != NULL){
                printf("Indirizzo: %s\n", inet_ntoa(temp->indirizzoIp.sin_addr));
                printf("Porta Congratulazioni: %d\n\n", htons(temp->portaMessaggiCongratulazioni));
                temp = (vincitore_t *)temp->next;
        }
         
        analisiSessionePuntata.numeroVincitori = contVincitori;
        analisiSessionePuntata.numeroPerdenti = contPerdenti;
        analisiSessionePuntata.stato = 1;
        Pthread_cond_broadcast(&analisiSessionePuntata.attesaMessaggi);
        Pthread_mutex_unlock(&analisiSessionePuntata.mutex);

        /*la lista delle puntate è stata processata completamente.
          la lista dei vincitori è stata riempita.
          ora il croupier può segnalare che ha concluso la gestione
          delle puntate*/

        contPerdenti = contVincitori = 0;
    }
}
