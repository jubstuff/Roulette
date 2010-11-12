#include "common_header.h"
#include"list_management.h"

/*TODO decidere quando sottrarre dalla somma del giocatore la cifra giocata*/

/*TODO decidere come devono essere legate tra loro le strutture dati*/

player_t *bet_tag_insert(player_t *player, bet_t *curr_bet, struct sockaddr_in client_addr) {
	queue_put(&(player->bet), (node *)curr_bet)
#ifdef OLD
	bet_t *b1; /* puntatore a lista ausiliario */
	player_t *p2; /* puntatore a lista ausiliario */
	int trovato;

	/* ricerchiamo il player nella lista dei giocatori e poi inseriamo nella 
	 * sual lista la puntata */

	//JUST_ASK perché dobbiamo cercare dove effettuare l'inserimento? O semplicemente controlla che il client sia già connesso?
	p2 = player;
	while ((trovato != 1) && (p2 != NULL)) {
		if ((p2->info_client->client_data.sin_addr.s_addr == client_addr.sin_addr.s_addr) && (p2->info_client->client_data.sin_port == client_addr.sin_port)) {
			trovato = 1;
		} else {
			trovato = 0;
			p2 = p2->next_player;
		}
	}
	if (trovato == 1) {
		if (p2->bet == NULL) {
			p2->bet = curr_bet;
			p2->bet->next_bet = NULL;
		} else { /* ricerca della posizione di inserimento */
			b1 = p2->bet;
			while (b1->next_bet != NULL) {
				b1 = b1->next_bet; /* scorre in avanti p1 */
			}
			curr_bet->next_bet = NULL;
			b1->next_bet = curr_bet; /* connessione all'elemento precedente */
		}
	}
	return (player);
#endif
}

void player_tag_insert(player_t *player, player_t *curr_player) {
	//In questa funzione dobbiamo inserire le cose peculiari del player
	queue_put(&player, (node *)curr_player);
#ifdef OLD
	player_t *p1; // puntatore a lista ausiliario
	if (player == NULL) {
		player = curr_player;
		player->next_player = NULL;
	} else {/* ricerca della posizione di inserimento */ //JUST_ASK perché effettuare la ricerca? non possiamo semplicemente inserirlo in testa?
		p1 = player;
		while (p1->next_player != NULL) {
			p1 = p1->next_player; /* scorre in avanti p1 */
		}
		curr_player->next_player = NULL;
		p1->next_player = curr_player; /* connessione all'elemento precedente */
	}
	return (player); //JUST_ASK perché ritorna il puntatore alla lista?
#endif

}

void player_tag_print(player_t *player) {
	while (player != NULL) {
		printf("{nickname : [%s] sommagioco : [%d] somma vinta : [%d] messportaggi : [%d] porta : [%d] ip : [%d] }", player->nickname, player->money, player->win_money, player->messport, player->info_client->client_data.sin_port, player->info_client->client_data.sin_addr.s_addr);
		bet_tag_print(player->bet);
		printf("---> ");
		player = player->next_player;
	}
	printf("NULL\n\n");
}

void bet_tag_print(bet_t *bet) {
	while (bet != NULL) {
		if (bet->bet_type == 1) {
			printf("{bet_num : [%d] bet_money : [%d]}", bet->bet_num, bet->bet_money);
			printf("---> ");
			bet = bet->next_bet;
		} else if (bet->bet_type == 2) {
			printf("{bet_num : [%c] bet_money : [%d]}", bet->bet_num_EU, bet->bet_money);
			printf("---> ");
			bet = bet->next_bet;
		}
	}
	printf("NULL\n\n");
}

void win_player(player_t *player, int bet_num_win) {
	char even_uneven; //FIXME dispari si dice odd
	bet_t *b0;

	if ((bet_num_win % 2) == 0) {
		even_uneven = 'P';//FIXME dispari si dice odd
	} else {
		even_uneven = 'D';//FIXME dispari si dice odd
	}
	while (player != NULL) { /* scorriamo la lista dei giocatori */
		b0 = player->bet;
		while (b0 != NULL) { /* scorriamo la lista delle puntante di ogni player */
			if ((b0->bet_type == 1) && (b0->bet_num == bet_num_win)) {
				printf("########## %s win !!! ##########\n", player->nickname);
				player->win_money = player->win_money + (36 * b0->bet_money);
			} else if ((b0->bet_type == 2) && (b0->bet_num_EU == even_uneven)) {//FIXME dispari si dice odd
				printf("########## %s win !!! ##########\n", player->nickname);
				player->win_money = player->win_money + (2 * b0->bet_money);
			}
			b0 = b0->next_bet;
		}
		player = player->next_player;
	}
}