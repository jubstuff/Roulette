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




#endif	/* CONTROL_H */

