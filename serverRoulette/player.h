/**
 * @file   player.h
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include la definizione del routine per il thread player e i suoi
 * sotto-thread
 *
 */

#include "queue.h"

#ifndef PLAYER_H
#define	PLAYER_H

/**
 * argomento
 *
 * argomento del gestore delle puntate nel server.
 *
 * @param listaPuntatePrivata Contiene la lista di puntate temporanea ricevuta
 * dal gestore
 * @param clientFd Il socket di comunicazione con il client associato al thread
 * player
 */
typedef struct argomento {
    queue *listaPuntatePrivata;
    int clientFd;
} argomento_gestore_puntate_t;
/**
 * player
 *
 * Routine per il thread player
 * @param arg Argomento del player thread. //TODO inserire cosa ci sta
 * @return
 */
void *player(void *arg);

/**
 *
 * @param arg Argomento per il gestore di puntate. Di tipo
 * argomento_gestore_puntate_t
 * @return
 */
void *gestorePuntateGiocatore(void *arg);

#endif	/* PLAYER_H */

