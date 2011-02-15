/**
 * @file   client_header.c
 * @Author Gruppo 7
 * @date   Gennaio 2011
 * @brief  Include tutte le definizioni per il client della Roulette
 *
 */
#include "client_header.h"
#include <stdio.h>
#include <ctype.h>


int parse_bet(char *puntataStr, int *sommaPuntata, int *tipoPuntata, int *numeroPuntato) {
    /* FIXME non c'è controllo sull'overflow dell'int dei soldi.
     * Non dovrebbe essere un grosso problema in ogni caso, perché
     * il controllo sul valore massimo della puntata si deve comunque
     * fare, anche se non in questa funzione */
    int error_flag = 0;
    char bet_type;

    *numeroPuntato = NO_NUMBER;

    if ((sscanf(puntataStr, "%d:%d", numeroPuntato, sommaPuntata)) == 2) {
        /* è stata inserita una puntata corretta del tipo <X>:<N> */
        if (*numeroPuntato >= 0 && *numeroPuntato <= 36) {
            //OK
            *tipoPuntata = *numeroPuntato;
        } else {
            //puntato un numero non corretto
            error_flag = ENUMRANGE;
        }
    } else if ((sscanf(puntataStr, "%c:%d", &bet_type, sommaPuntata)) == 2) {
        /* è stata inserita una puntata del tipo char:<N>*/
        bet_type = toupper(bet_type);
        if (bet_type == 'P') {
            //caso P:<N>
            *tipoPuntata = -2;
        } else if (bet_type == 'D') {
            //caso D:<N>
            *tipoPuntata = -1;
        } else {
            //carattere non valido
            error_flag = ECHARINV;
        }
    } else {
        //sintassi puntata non valida
        error_flag = EBADSYNTAX;
    }
    return error_flag;
}

char *tipoPuntataTestuale(int tipo) {
    if (tipo == -1) {
        return "Dispari";
    } else if (tipo == -2) {
        return "Pari";
    } else if (tipo >= 0 && tipo <= 36) {
        return "Numero";
    } else {
        return "Puntata non valida";
    }
}
