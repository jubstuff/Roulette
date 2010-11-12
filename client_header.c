#include "client_header.h"

#define ENUMRANGE -1
#define ECHARINV -2
#define EBADSYNTAX -3

/**
 * parse_bet
 *
 * Analizza una puntata. Riconosce se è stata fatta una puntata di tipo
 * Pari/Dispari oppure su un numero
 * */
int parse_bet(char *bet, int *money_bet, char *bet_type, int *number_bet) {
	/* FIXME non c'è controllo sull'overflow dell'int dei soldi.
	 * Non dovrebbe essere un grosso problema in ogni caso, perché
	 * il controllo sul valore massimo della puntata si deve comunque
	 * fare, anche se non in questa funzione */
	int error_flag = 0;

	if ( (sscanf(bet, "%d:%d", number_bet, money_bet)) == 2) {
		/* è stata inserita una puntata corretta del tipo <X>:<N> */
		if( *number_bet >= 0 && *number_bet <= 36 ) {
			//OK
			printf("Numero\n");
			printf("Sono stati puntati %d€ sul %d\n", *money_bet, *number_bet);
		} else {
			//puntato un numero non corretto
			//printf("Errore: bisogna puntare un numero 0 <= N <= 36\n");
			error_flag = ENUMRANGE;
		}
	}
	else if ( (sscanf(bet, "%c:%d", bet_type, money_bet)) == 2) {
		/* è stata inserita una puntata del tipo char:<N>*/
		*bet_type = toupper(*bet_type);
		if ( *bet_type == 'P' ) {
			//caso P:<N>
			printf("Pari\n");
			printf("Sono stati puntati %d€ sui pari\n", *money_bet);
		}
		else if( *bet_type == 'D' ) {
			//caso D:<N>
			printf("Dispari\n");
			printf("Sono stati puntati %d€ sui dispari\n", *money_bet);
		}
		else {
			//carattere non valido
			//printf("Puntata non valida: il carattere è errato\n");
			error_flag = ECHARINV;
		}
	}
	else {
		//sintassi puntata non valida
		//printf("Puntata non valida: errore sintassi\n");
		error_flag = EBADSYNTAX;
	}
	return error_flag;
}	
