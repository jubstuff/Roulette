/* 
 * File:   player.h
 * Author: just
 *
 * Created on 22 novembre 2010, 20.59
 */

#ifndef PLAYER_H
#define	PLAYER_H

struct argomentoGestorePuntate {
    queue *listaPuntatePrivata;
    int clientFd;
};

void *player(void *arg);
void *gestore_puntate(void *arg);

#endif	/* PLAYER_H */

