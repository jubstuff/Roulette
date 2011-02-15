/**
 * @file   client_header.h
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include tutte le definizioni per il client della Roulette
 *
 */

#ifndef CLIENT_HEADER_H
#define	CLIENT_HEADER_H

#include "../common/common_header.h"
/**
 * Errore di parse_bet: il numero è oltre il range corretto (0-36)
 */
#define ENUMRANGE -1
/**
 * Errore di parse_bet: è stato inserito un tipo puntata non valido, ovvero
 * diverso da P/D/<N>
 */
#define ECHARINV -2
/**
 * Errore di parse_bet: la sintassi di puntata non è corretta
 */
#define EBADSYNTAX -3
/**
 * Placeholder per parse_bet, inizializza il numero a un valore esterno al
 * range, per precauzione
 */
#define NO_NUMBER -4
/**
 * Limite massimo per il budget dei giocatori
 */
#define LIMITE_BUDGET 1000

/**
 * lettorePuntate
 *
 * Routine per il thread lettorePuntate
 * @param arg Argomento del player thread. //TODO inserire cosa ci sta
 * @return
 */
void *lettorePuntate(void *arg);
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

/**
 * gestisciMessaggiVittoria
 *
 * Riceve tutti i messaggi di congratulazione dai giocatori che hanno perso e
 * li salva in bufRisultato
 *
 * @param serverFd Socket di comunicazione con il server
 * @param clientFd Socket su cui il client è in ascolto per i messaggi di
 * congratulazione
 * @param bufRisultato Stringa contenente tutti i messaggi di congratulazione
 */
void gestisciMessaggiVittoria(int serverFd, int clientFd, char *bufRisultato);

/**
 * gestisciMessaggiPerdita
 *
 * Invia i messaggi di congratulazione a tutti i giocatori che hanno vinto
 *
 * @param serverFd Socket di comunicazione con il server
 * @param nickname Nickname del giocatore corrente
 */
void gestisciMessaggiPerdita(int serverFd, char *nickname);
#endif	/* CLIENT_HEADER_H */

