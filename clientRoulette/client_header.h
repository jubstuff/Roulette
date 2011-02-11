/* 
 * File:   client_header.h
 * Author: just
 *
 * Created on 12 novembre 2010, 23.12
 */

#ifndef CLIENT_HEADER_H
#define	CLIENT_HEADER_H
/**
 * parse_bet
 *
 * Analizza una puntata. Riconosce se è stata fatta una puntata di tipo
 * Pari/Dispari oppure su un numero
 * */
int parse_bet(char *puntataStr, int *sommaPuntata, int *tipoPuntata, int *numeroPuntato);
char *tipoPuntataTestuale(int tipo);
#endif	/* CLIENT_HEADER_H */

