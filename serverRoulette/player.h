/* 
 * File:   player.h
 * Author: just
 *
 * Created on 22 novembre 2010, 20.59
 */

#include "queue.h"

#ifndef PLAYER_H
#define	PLAYER_H

typedef struct argomento {
    queue *listaPuntatePrivata;
    int clientFd;
} argomento_gestore_puntate_t;

void *player(void *arg);
void *gestorePuntateGiocatore(void *arg);

#endif	/* PLAYER_H */

