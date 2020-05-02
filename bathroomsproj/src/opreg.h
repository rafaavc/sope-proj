#include <pthread.h>

enum OPERATION {
    IWANT,  // - cliente faz pedido inicial
    RECVD,  // - servidor acusa receção de pedido
    ENTER,  // - servidor diz que aceitou pedido
    IAMIN,  // - cliente acusa a utilização do Quarto de Banho
    TIMUP,  // - servidor diz que terminou o tempo de utilização
    TLATE,  // [2LATE]!! - servidor rejeita pedido por Quarto de Banho já ter encerrado
    CLOSD,  // - cliente acusa informação de que o Quarto de Banho está fechado
    FAILD,  // - cliente já não consegue receber resposta do servidor
    GAVUP,  // - servidor já não consegue responder a pedido porque FIFO privado do cliente fechou
};

/**
 * @brief Log operations to fd
 * 
 * @param i número sequencial do pedido
 * @param pid identificador de sistema do processo
 * @param tid identificador no sistema do thread
 * @param dur duração, em milissegundos, de utilização do Quarto de Banho
 * @param pl nº de lugar que eventualmente lhe será atribuído no Quarto de Banho 
 * @param oper operação que o processo acabou de executar
 * @param n number of file descriptors
 * 
 */
void logOperation(int i, pid_t pid, pthread_t tid, int dur, int pl, enum OPERATION oper, int n, ...);

/**
 * @brief Separete log parameters
 * 
 * @param string string from logOperation
 * @param t time
 * @param i número sequencial do pedido
 * @param pid identificador de sistema do processo
 * @param tid identificador no sistema do thread
 * @param dur duração, em milissegundos, de utilização do Quarto de Banho
 * @param pl nº de lugar que eventualmente lhe será atribuído no Quarto de Banho 
 * @param oper operação que o processo acabou de executar
 */
void receiveLogOperation(char *string, long *t, int *i, pid_t *pid, pthread_t *tid, int *dur, int *pl , enum OPERATION *oper);