typedef struct client_tag {
	struct sockaddr_in	client_data;	/* porta,indirizzo del client */
	int 				clientfd;		/* socket del client */
}client_t;

/* struttura contenente i dati di una singola giocata */
typedef struct bet_tag {
	int bet_type; /* flag che assume valore 1 se si punta su un bet_num scelto, 2 se si 
	               * punta su un bet_num pari o dispari */
	int bet_num;
	char bet_num_EU;/*even uneven (pari o dispari)*/
	int bet_money;
	struct bet_tag *next_bet;/*prossimo elemento*/
}bet_t;

/* struttura contenente i dati di un singolo player */
typedef struct player_tag {
	int money;
	char nickname[50];
	int win_money;
	int messport;
	client_t *info_client;
	bet_t *bet;/*lista bet del player*/
	struct player_tag *next_player;/*prossimo elemento*/
}player_t;

/* ----------------------------------------------------------------*/
player_t *bet_tag_insert(player_t *, bet_t *, struct sockaddr_in );
void bet_tag_print(bet_t *);
player_t *player_tag_insert(player_t * , player_t *);
void player_tag_print(player_t * );
void win_player(player_t *, int);
/* ----------------------------------------------------------------*/