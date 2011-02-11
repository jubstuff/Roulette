#include "common_header.h"

//int num_requests = 0;
/**
 * Costanti
 */
const char messaggioPuntateAperte[] = "\n=Puntate aperte=\n";
ssize_t lenMessaggioPuntateAperte = sizeof (messaggioPuntateAperte);
const char messaggioPuntateChiuse[] = "\n=Puntate chiuse=\n";
ssize_t lenMessaggioPuntateChiuse = sizeof (messaggioPuntateChiuse);

int numeroMinimoGiocatori = 2;

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


/**
 * Apre un socket, e si mette in ascolto su di esso
 * @param self
 * @param server_port
 * @return il socket aperto
 */
int open_socket(struct sockaddr_in self, short int server_port) {
    int sockfd;
    /* apro il socket */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* preparo la struct con le informazioni del server */
    bzero(&self, sizeof (self));
    /* protocol family in host order */
    self.sin_family = AF_INET;
    self.sin_port = htons(server_port);
    self.sin_addr.s_addr = htonl(INADDR_ANY);

    /* collego il socket */
    Bind(sockfd, (struct sockaddr *) &self, sizeof (self));
    /* pone il server in ascolto */
    Listen(sockfd, 5); //TODO quanti ce ne devono stare nella listen?
    return sockfd;
}

/**
 * Controlla se una puntata del tipo <N>:<M> è vincente
 * @param estratto
 * @param puntata
 * @param player
 */
void gestisci_puntata_numero(int estratto, puntata_t *puntata, player_t *player) {
    if (estratto == puntata->numeroPuntato) {
        aumenta_budget(36, puntata, player); //TODO modificare moltiplicatore con costante
    }
}

/**
 * Controlla se una puntata del tipo P:<N> è vincente
 * @param estratto
 * @param puntata
 * @param player
 */
void gestisci_puntata_pari(int estratto, puntata_t *puntata, player_t *player) {
    if (estratto % 2 == 0) {
        aumenta_budget(2, puntata, player); //TODO modificare moltiplicatore con costante
    }
}

/**
 * Controlla se una puntata del tipo D:<N> è vincente
 * @param estratto
 * @param puntata
 * @param player
 */
void gestisci_puntata_dispari(int estratto, puntata_t *puntata, player_t *player) {
    if (estratto % 2 != 0) {
        aumenta_budget(2, puntata, player); //TODO modificare moltiplicatore con costante
    }
}

/**
 * Aumenta il budget del giocatore, dato un moltiplicatore
 * @param moltiplicatore
 * @param puntata
 * @param player
 */
void aumenta_budget(int moltiplicatore, puntata_t *puntata, player_t *player) {
    player->budgetAttuale += (puntata->sommaPuntata * moltiplicatore);
}

/**
 * Calcola l'intervallo di attesa del croupier
 * @param intervallo
 * @return la struttura timespec contenente il tempo di fine attesa
 */
struct timespec calcola_intervallo(int intervallo) {
    time_t now;
    struct timespec cond_time;
    now = time(NULL);
    cond_time.tv_sec = now + intervallo;
    cond_time.tv_nsec = 0;
    return cond_time;
}

/******************************************************************************
 * =WRAPPER FUNCTIONS=
 */

int Socket(int domain, int type, int protocol) {
    int sockFd;

    sockFd = socket(domain, type, protocol);
    if (sockFd < 0) {
        err_abort(errno, "Errore nella creazione del socket");
    }
    return sockFd;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int status;
    status = bind(sockfd, addr, addrlen);
    if (status != 0) {
        err_abort(errno, "Errore nella bind");
    }
}

void Listen(int sockfd, int backlog) {
    int status;
    status = listen(sockfd, backlog);
    if (status != 0) {
        err_abort(errno, "Errore nella listen");
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int socket;
    socket = accept(sockfd, addr, addrlen);
    if (socket < 0) {
        err_abort(errno, "Errore nell'accept");
    }
    return socket;
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int status;
    status = connect(sockfd, addr, addrlen);
    if (status != 0) {
        err_abort(errno, "Errore nella connessione: riavvia il programma.");
    }
}

void Close(int fildes){
    int status;
    status = close(fildes);
    if(status < 0){
        err_abort(errno, "Errore nella close");
    }
}

//TODO controllare questa
void *Malloc(size_t size){
    void * ptr;
    ptr = malloc(size);
    if(ptr == NULL){
        err_abort(errno, "Errore nella malloc");
    }
    return ptr;
}

void Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
    int status;
    status = pthread_mutex_init(mutex, attr);
    if(status != 0){
        err_abort(status, "Errore inizializzazione mutex");
    }
}

void Pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr){
    int status;
    status = pthread_cond_init(cond, attr);
    if(status != 0){
        err_abort(status, "Errore inizializzazione condition variable");
    }
}

void Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
        void *(*start_routine)(void*), void *arg) {
    int status;
    status = pthread_create(thread, attr, start_routine, arg);
    if(status != 0){
        err_abort(status, "Errore nella pthread_create");
    }
}

void Pthread_cancel(pthread_t thread){
    int status;
    status = pthread_cancel(thread);
    if(status != 0){
        err_abort(status, "Errore nella pthread_cancel");
    }
}

void Pthread_mutex_lock(pthread_mutex_t *mutex){
    int status;
    status = pthread_mutex_lock(mutex);
    if(status != 0){
        err_abort(status, "Errore nella pthread_mutex_lock");
    }
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex){
    int status;
    status = pthread_mutex_unlock(mutex);
    if(status != 0){
        err_abort(status, "Errore nella pthread_mutex_unlock");
    }
}

void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
    int status;
    status = pthread_cond_wait(cond, mutex);
    if(status != 0){
        err_abort(status, "Errore nella pthread_cond_wait");
    }
}

int Pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
        const struct timespec *abstime){
    int status;
    status = Pthread_cond_timedwait(cond, mutex, abstime);
    if (status == ETIMEDOUT) {
        return status;
    } else if (status != 0) {
        err_abort(status, "Errore nella pthread_cond_timedwait");
    }
    return status;
}

void Pthread_cond_broadcast(pthread_cond_t *cond) {
    int status;
    status = pthread_cond_broadcast(cond);
    if(status != 0){
        err_abort(status, "Errore nella pthread_cond_broadcast");
    }
}

void Pthread_cond_signal(pthread_cond_t *cond){
    int status;
    status = pthread_cond_signal(cond);
    if(status != 0){
        err_abort(status, "Errore nella pthread_cond_signal");
    }
}

ssize_t Write(int fd, const void *buf, size_t count) {
    ssize_t bytes_written;
    bytes_written = write(fd, buf, count);
    if(bytes_written < 0) {
        err_abort(errno, "Errore nella write");
    }
    return bytes_written;
}

ssize_t Read(int fd, void *buf, size_t count) {
    ssize_t bytes_read;
    bytes_read = read(fd, buf, count);
    if(bytes_read < 0){
        err_abort(errno, "Errore nella read");
    }
    return bytes_read;
}
