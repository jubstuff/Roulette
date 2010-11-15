/* 
 * File:   control.h
 * Author: just
 *
 * Created on 9 novembre 2010, 23.36
 */

#ifndef CONTROL_H
#define	CONTROL_H

#include <pthread.h>

typedef struct data_control {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int active;
} data_control;

int control_init(data_control *mycontrol);
int control_destroy(data_control *mycontrol);
int control_activate(data_control *mycontrol );
int control_deactivate(data_control *mycontrol);

#endif	/* CONTROL_H */

