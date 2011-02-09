#include "common_header.h"

int num_requests = 0;
/**
 * Costanti
 */
const char messaggioPuntateAperte[] = "\n=Puntate aperte=\n";
ssize_t lenMessaggioPuntateAperte = sizeof (messaggioPuntateAperte);

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
 * roulette_mutex_lock
 * 
 * Effettua il lock sul mutex ed il check per errori
 * 
 * @param mutex mutex da bloccare
 * @param msg messaggio da visualizzare
 * @return il valore restituito pthread_mutex_lock
 */
int roulette_mutex_lock(pthread_mutex_t *mutex, char *msg) {
    int status;
    status = pthread_mutex_lock(mutex);
    if (status != 0) {
        err_abort(status, msg);
    }
    return status;
}

/**
 * roulette_mutex_unlock
 *
 * Effettua l'unlock sul mutex ed il check per errori
 *
 * @param mutex mutex da sbloccare
 * @param msg messaggio da visualizzare
 * @return il valore restituito pthread_mutex_lock
 */
int roulette_mutex_unlock(pthread_mutex_t *mutex, char *msg) {
    int status;
    status = pthread_mutex_unlock(mutex);
    if (status != 0) {
        err_abort(status, msg);
    }
    return status;
}

/**
 * Apre un socket, e si mette in ascolto su di esso
 * @param self
 * @param server_port
 * @return il socket aperto
 */
int open_socket(struct sockaddr_in self, short int server_port) {
    int sockfd;
    int status;
    /* apro il socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_abort(errno, "Creazione socket");
    }

    /* preparo la struct con le informazioni del server */
    bzero(&self, sizeof (self));
    /* protocol family in host order */
    self.sin_family = AF_INET;
    self.sin_port = htons(server_port);
    self.sin_addr.s_addr = htonl(INADDR_ANY);

    /* collego il socket */
    status = bind(sockfd, (struct sockaddr *) &self, sizeof (self));
    if (status != 0) {
        err_abort(errno, "Bind socket");
    }
    /* pone il server in ascolto */
    status = listen(sockfd, 5); //TODO quanti ce ne devono stare nella listen?
    if (status != 0) {
        err_abort(errno, "Error in listening to socket");
    }
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

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
        void *(*start_routine)(void*), void *arg) {
    pthread_create(thread, attr, start_routine, arg);
}

#ifdef ASDRUBALE_BARCA

/**
 * Processa la lista dei giocatori, contando il numero di vincitori e perdenti,
 * e aumentando il budget dei vincitori
 *
 * LOCKING PROTOCOL questa funzione necessita dei mutex puntate e player bloccati
 * @param estratto numero estratto dalla roulette
 */
void gestisci_puntate(int estratto) {
    puntata_t *puntata;
    queue *q = &(players_list.giocatori);
    int vincitore = 0;
    int contatore = 0;
    player_t *player = (player_t *) (q->head);
    int numero_di_perdenti_in_questa_mano = 0;
    int numero_di_vincitori_in_questa_mano = 0;
    int budget_giocatore_prima_del_controllo = 0;

    printf("CROUPIER Il numero estratto è %d\n", estratto);
    while (player != NULL) {
        printf("===GIOCATORE %d===\n", contatore);
        budget_giocatore_prima_del_controllo = player->money;
        while ((puntata = (puntata_t *) queue_get(&(player->lista_puntate_personale.puntate))) != NULL) {
            switch (puntata->tipo) {
                case NUMBER:
                    gestisci_puntata_numero(estratto, puntata, player);
                    break;
                case EVEN:
                    gestisci_puntata_pari(estratto, puntata, player);
                    break;
                case ODD:
                    gestisci_puntata_dispari(estratto, puntata, player);
                    break;
                default:
                    break;
            }
        }
        vincitore = (player->money > budget_giocatore_prima_del_controllo) ? 1 : 0;
        contatore++;
        if (vincitore) {
            //TODO aggiungi alla lista dei vincitori
            numero_di_vincitori_in_questa_mano++;
        } else {
            numero_di_perdenti_in_questa_mano++;
        }
        if (player->money == 0) {
            //TODO aggiungi alla lista dei giocatori da eliminare dal gioco
        }

        player = (player_t *) player->next;
    }
    printf("CROUPIER In questa mano ci sono stati %d vincitori e %d perdenti\n",
            numero_di_vincitori_in_questa_mano, numero_di_perdenti_in_questa_mano);
    sleep(10);
    //TODO inviare il numero di perdenti a tutti i client
    //TODO inviare gli indirizzi IP dei vincitori a tutti i client
}

/**
 * Inizializza un nodo della lista puntate
 * @param numero_puntato
 * @param tipo_puntata
 * @param somma_puntata
 * @return
 */
puntata_t *inizializza_nodo_puntata(int numero_puntato, bet_t tipo_puntata, int somma_puntata) {
    puntata_t *mybet;
    mybet = (puntata_t *) malloc(sizeof (puntata_t));
    if (!mybet) {
        err_abort(errno, "Errore malloc!");
    }
    mybet->numero = numero_puntato;
    mybet->tipo = tipo_puntata;
    mybet->somma_puntata = somma_puntata;
    return mybet;
}

#endif

