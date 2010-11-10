/* control.c
**
** These routines provide an easy way to make any type of
** data-structure thread-aware.  Simply associate a data_control
** structure with the data structure (by creating a new struct, for
** example).  Then, simply lock and unlock the mutex, or
** wait/signal/broadcast on the condition variable in the data_control
** structure as needed.
*/
#include "control.h"

int control_init(data_control *mycontrol) {
	int status;
	status = pthread_mutex_init(&(mycontrol->mutex), NULL);
	if(status != 0){
		return 1;
	}

	status = pthread_cond_init(&(mycontrol->cond), NULL);
	if(status != 0){
		return 1;
	}

	mycontrol->active = 0;
	return 0;
}

int control_destroy(data_control *mycontrol) {
	int status;

	status = pthread_mutex_destroy(&(mycontrol->mutex));
	if(status != 0){
		return 1;
	}

	status = pthread_cond_destroy(&(mycontrol->cond));
	if(status != 0){
		return 1;
	}

	mycontrol->active = 0;
	return 0;
}

int control_activate(data_control *mycontrol ){
	int status;

	status = pthread_mutex_lock(&(mycontrol->mutex));
	if(status != 0) {
		return 0;
	}
	mycontrol->active = 1;
	status = pthread_mutex_unlock(&(mycontrol->mutex));
	pthread_cond_broadcast(&(mycontrol->cond));
	return 1;
}

int control_deactivate(data_control *mycontrol) {
	int status;

	status = pthread_mutex_lock(&(mycontrol->mutex));
	if(status != 0) {
		return 0;
	}
	mycontrol->active = 0;
	status = pthread_mutex_unlock(&(mycontrol->mutex));
	pthread_cond_broadcast(&(mycontrol->cond));
	return 1;
}


