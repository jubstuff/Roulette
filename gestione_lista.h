/* struttura contenente i dati di una singola giocata */
typedef struct lista_puntate {
	int tipo_giocata; /* flag che assume valore 1 se si punta su un numero scelto, 2 se si 
	                   * punta su un numero pari o dispari */
	int numero;
	char numero_PD;
	int cifra;
	struct lista_puntate *ptr_lista;
}LISTA_puntate;

/* struttura contenente i dati di un singolo giocatore */
typedef struct lista_giocatori {
	int somma_gioco;
	char *nickname;
	int somma_vinta;
	int portamess;
	in_addr_t indirizzo_ip;
	LISTA_puntate *puntate;
	struct lista_giocatori *ptr_lista;
}LISTA_giocatori;


//########################################################################################################
LISTA_giocatori *inserimento_lista_puntate(LISTA_giocatori *, in_addr_t, int , int , int , char , int );
LISTA_puntate *eliminazione_lista_puntate(LISTA_puntate *, int );
void stampa_lista_puntate(LISTA_puntate *);
LISTA_giocatori *inserimento_lista_giocatori(LISTA_giocatori * , int , char * , int , in_addr_t );
void stampa_lista_giocatori(LISTA_giocatori * );
//########################################################################################################
