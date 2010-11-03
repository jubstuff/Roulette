#include"gestione_lista.h"

LISTA_giocatori *inserimento_lista_puntate(LISTA_giocatori *p, in_addr_t indirizzo_ip, int portamess, int tipo_giocata, int numero, char numero_PD, int cifra) {
	LISTA_puntate *p0,/* puntatore a lista ausiliario */
	              *p1;/* puntatore a lista ausiliario */
        LISTA_giocatori *p2;
	int trovato;
    
    /* ricerchiamo il giocatore nella lista dei giocatori e poi inseriamo nella 
	 * sual lista la puntata */
	p2=p;
	while(trovato != 1 && (p2 != NULL)) {
		if((p2->indirizzo_ip == indirizzo_ip) && (p2->portamess == portamess)) {
			trovato=1;
		}
		else {
			trovato=0;
			p2=p2->ptr_lista;
		}
	}
	if(trovato == 1) {
		p0=(LISTA_puntate *)malloc(sizeof(LISTA_puntate));
		if(tipo_giocata == 1) {
			p0->tipo_giocata=tipo_giocata;
			p0->numero=numero;
			p0->cifra=cifra;
		}
		else if(tipo_giocata == 2) {
			p0->tipo_giocata=tipo_giocata;
			p0->numero_PD=numero_PD;
			p0->cifra=cifra;
		}
		if(p2->puntate == NULL) {
			p2->puntate=p0;
			p2->puntate->ptr_lista=NULL;
		}
		else {  /* ricerca della posizione di inserimento */
			p1=p2->puntate;
			while(p1->ptr_lista != NULL) {
				p1=p1->ptr_lista; /* scorre in avanti p1 */
			}
			p0->ptr_lista=NULL;
			p1->ptr_lista=p0;/* connessione all'elemento precedente */
		}
	}
return(p);     
}

LISTA_puntate *eliminazione_lista_puntate(LISTA_puntate *p, int ele) {
	LISTA_puntate *p_1,/* puntatore a lista ausiliario */
                  *p_2,/* puntatore a lista ausiliario */
                  *app;/* puntatore a lista ausiliario */
                  int trovato=0;/* variabile che assume il valore "1" se è stato trovato l'elemento 
                                 * all'interno della lista, "0" altrimenti */
    p_1=p;
	
    if(p_1!=NULL) { /*se la lista è vuota fine*/
		/*se è il primo da eliminare*/
		if(p_1->tipo_giocata==ele) {
			p_2=p_1;
			p=p->ptr_lista;/*si modifica il puntatore alla testa della lista*/
			free(p_2);
			app=p;
		}
		else {    /*ricerca dell'elemento da eliminare*/
			while(p_1->ptr_lista != NULL && trovato != 1) {
				if(p_1->ptr_lista->tipo_giocata != ele) {
					p_1=p_1->ptr_lista;  /*scorre in avanti p_1*/
				}
				else {
					trovato=1; /*interrompe lo scorrimento*/
                    p_2=p_1->ptr_lista;
                    p_1->ptr_lista=p_1->ptr_lista->ptr_lista;/*eliminazione elemento*/
                    free(p_2);
                    app=p;
				}
			}
		}
	}
	if(trovato==0) {
		/* elemento non presente nella lista */
		app=p;
	}
return app;
}

void stampa_lista_puntate(LISTA_puntate *p) {
	while(p != NULL) {
		if(p->tipo_giocata == 1) {
			printf("{numero : [%d] cifra : [%d]}",p->numero,p->cifra);
			printf("---> ");
			p=p->ptr_lista;
		}
		else if(p->tipo_giocata == 2) {
			printf("{numero : [%c] cifra : [%d]}",p->numero_PD,p->cifra);
			printf("---> ");
			p=p->ptr_lista;
		}
	}
	printf("NULL\n\n");
}

LISTA_giocatori *inserimento_lista_giocatori(LISTA_giocatori *p , int somma_gioco , char *nickname , int portamess , in_addr_t indirizzo_ip) {
	LISTA_giocatori *p0,/* puntatore a lista ausiliario */
	                *p1;/* puntatore a lista ausiliario */
    
    p0=(LISTA_giocatori *)malloc(sizeof(LISTA_giocatori));
	p0->somma_gioco=somma_gioco;
	p0->somma_vinta=0;
	p0->nickname=(char *)malloc(strlen(nickname)*sizeof(char));
	strcpy(p0->nickname,nickname);
	p0->portamess=portamess;
	p0->indirizzo_ip=indirizzo_ip;
	p0->puntate=NULL;
    if(p == NULL) {
		p=p0;
        p->ptr_lista=NULL;
	}
	else {  /* ricerca della posizione di inserimento */
		p1=p;
		while(p1->ptr_lista != NULL) {
			p1=p1->ptr_lista; /* scorre in avanti p1 */
		}
		p0->ptr_lista=NULL;
		p1->ptr_lista=p0;/* connessione all'elemento precedente */
	}
return(p);     
}

void stampa_lista_giocatori(LISTA_giocatori *p) {
	while(p != NULL) {
		printf("{nickname : [%s] sommagioco : [%d] somma vinta : [%d] portamessaggi : [%d] ip : [%d]}",p->nickname,p->somma_gioco,p->somma_vinta,p->portamess,p->indirizzo_ip);
		stampa_lista_puntate(p->puntate);
		printf("---> ");
		p=p->ptr_lista;
	}
	printf("NULL\n\n");
}