#include "opreg.h"
#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define MAX_STRING_SIZE 512

char * opStrings[] = {
    "IWANT",  // - cliente faz pedido inicial
    "RECVD",  // - servidor acusa receção de pedido
    "ENTER",  // - servidor diz que aceitou pedido
    "IAMIN",  // - cliente acusa a utilização do Quarto de Banho
    "TIMUP",  // - servidor diz que terminou o tempo de utilização
    "2LATE",  // - servidor rejeita pedido por Quarto de Banho já ter encerrado
    "CLOSD",  // - cliente acusa informação de que o Quarto de Banho está fechado
    "FAILD",  // - cliente já não consegue receber resposta do servidor
    "GAVUP",  // - servidor já não consegue responder a pedido porque FIFO privado do cliente fechou
};

/*
■ i - o número sequencial do pedido (gerado por Un)
■ pid - identificador de sistema do processo (cliente, no caso do pedido; servidor, no caso da resposta)
■ tid - identificador no sistema do thread cliente (cliente, no caso do pedido; servidor, no caso da
resposta)
■ dur - duração, em milissegundos, de utilização (de um lugar) do Quarto de Banho (valor atribuído
no pedido e repetido na resposta, se se der a ocupação; se não se der, por motivo de o Quarto de
Banho estar em vias de encerrar, o servidor responde aqui com o valor -1
■ pl – no de lugar que eventualmente lhe será atribuído no Quarto de Banho (no pedido, este campo é
preenchido com o valor -1 e na resposta terá o valor do lugar efetivamente ocupado ou também -1,
na sequência de insucesso de ocupação, por motivo de encerramento)
*/



void logOperation(int i, pid_t pid, pthread_t tid, int dur, int pl, enum OPERATION oper, int n, ...) {
    char * op = malloc(MAX_STRING_SIZE);
    sprintf(op, "%ld ; %d ; %d ; %lu ; %d ; %d ; %s\n", time(NULL), i, pid, tid, dur, pl, opStrings[oper]);

    va_list fds;
    va_start(fds, n);
    for (int i = 0; i < n; i++) {
        int fd = va_arg(fds, int);
        write(fd, op, strlen(op));
    }
    va_end(fds);
    free(op);
}

void receiveLogOperation(char *string, long *t, int *i, pid_t *pid, pthread_t *tid, int *dur, int *pl , enum OPERATION *oper){
    char * stok = strtok(string, ";");

    *t = atol(stok);
    stok = strtok(NULL, ";");
    *i = atoi(stok);
    stok = strtok(NULL, ";");
    *pid = atoi(stok);
    stok = strtok(NULL, ";");
    *tid = atol(stok);
    stok = strtok(NULL, ";");
    *dur = atoi(stok);
    stok = strtok(NULL, ";");
    *pl = atoi(stok);
    stok = strtok(NULL, ";");

    for (int i = 0; i < 9; i++){
        if (strstr(stok, opStrings[i]) != NULL){
            *oper = i;
        }
    }
}