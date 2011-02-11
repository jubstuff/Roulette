/**
 * @file   client_header.h
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include tutte le definizioni per il client della Roulette
 *
 */

#ifndef CLIENT_HEADER_H
#define	CLIENT_HEADER_H

/**
 * parse_bet
 *
 * Analizza una puntata. Riconosce se è stata fatta una puntata di tipo
 * Pari/Dispari oppure su un numero
 * 
 * @param puntataStr L'input dell'utente da linea di comando. Deve essere uno di
 * questi tre tipi:
 * - D:<X>;
 * - P:<X>;
 * - <X>:<Y>
 * @param sommaPuntata [out] Somma puntata dal giocatore
 * @param tipoPuntata [out] Tipo di puntata effettuata
 * @param numeroPuntato [out] Numero puntato dal giocatore
 * @return
 */
int parse_bet(char *puntataStr, int *sommaPuntata, int *tipoPuntata, int *numeroPuntato);

/**
 * tipoPuntataTestuale
 *
 * Restituisce la stringa relativa al tipo di puntata
 *
 * @param tipo Può assumere i valori:
 *  - -1 -> Dispari
 *  - -2 -> Pari
 *  - >=0 <=36 -> Numerico
 * @return La stringa corrispondente al tipo di puntata
 */
char *tipoPuntataTestuale(int tipo);
#endif	/* CLIENT_HEADER_H */

