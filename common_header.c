#include "common_header.h"



struct request *first_request = NULL; //testa della lista
struct request *last_request = NULL; //ultimo elemento della lista
int num_requests = 0;

/**
 * Stampa sullo standard error un messaggio contenente l'errore, il file che l'ha
 * causato, la riga e la spiegazione.
 * 
 * @param code il codice d'errore (il valore di ritorno per le funzioni pthread,
 *		la variabile errno per le altre syscall)
 * @param text Una stringa che spiega l'errore
 */

void err_abort(int code, char *text) {
	fprintf(stderr, "%s at %s:%d: %s\n", text, __FILE__, __LINE__, strerror(code));
	abort();
}

//void add_request(int request_num, pthread_mutex_t *mutex, pthread_cond_t *cond) {
void add_request(int request_num) {	
	int status; //codice di ritorno delle funzioni pthread
	struct request *a_request;

	/* crea un record con una nuova richiesta */
	a_request = (struct request *) malloc(sizeof (struct request));
	if (!a_request) {
		err_abort(-1, "add_request: memoria esaurita");
	}
	a_request->number = request_num;
	a_request->next = NULL;
	
#ifdef ASDRUBALE
	/* lock sul mutex per garantire accesso esclusivo alla lista */
	status = pthread_mutex_lock(mutex);
	if (status) {
		err_abort(status, "add_request: lock sul mutex fallito");
	}
#endif

	/* aggiunge una nuova richiesta alla fine della lista */
	if (num_requests == 0) {
		//caso speciale - lista vuota
		first_request = a_request;
		last_request = a_request;
	} else {
		last_request->next = a_request;
		last_request = a_request;
	}

	/* aumenta il numero di richieste in coda */
	num_requests++;

#ifdef DEBUG
	printf("add_request: aggiunta richiesta con id %d\n", a_request->number);
#endif

#ifdef ASDRUBALE
	/* unlock del mutex */
	status = pthread_mutex_unlock(mutex);
	if (status) {
		err_abort(status, "add_request: unlock sul mutex fallito");
	}
	/* c'è una nuova richiesta da gestire */
	status = pthread_cond_signal(cond);
	if (status) {
		err_abort(status, "add_request: signal nuova richiesta fallito");
	}
#endif
} //fine add_request

struct request *get_request() {
	struct request *a_request;

	if (num_requests > 0) {
		a_request = first_request;
		first_request = a_request->next;
		if (first_request == NULL) {
			last_request = NULL;
		}
		/* decrementa il numero di richieste */
		num_requests--;
	} else {
		/* lista vuota */
		a_request = NULL;
	}


	return a_request;

}


void
handle_request(struct request* a_request, pthread_t thread_id) {
	if (a_request) {
		printf("Thread '%d' handled request '%u'\n",
			(unsigned int)thread_id, a_request->number);
	}
}


