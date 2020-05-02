#include <pthread.h>
#include <stdbool.h>

#define NOFD -1

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

struct op_struct {
    pid_t pid;
    pthread_t tid;
    int i, dur, pl;
    enum OPERATION oper;
};
typedef struct op_struct structOp;

/**
 * @brief Log operations to fd
 * 
 * @param i número sequencial do pedido
 * @param pid identificador de sistema do processo
 * @param tid identificador no sistema do thread
 * @param dur duração, em milissegundos, de utilização do Quarto de Banho
 * @param pl nº de lugar que eventualmente lhe será atribuído no Quarto de Banho 
 * @param oper operação que o processo acabou de executar
 * @param writeToSTDOUT whether to write the operation to stdout
 * @param fd fd of where to write the structOp
 * 
 */
void logOperation(int i, pid_t pid, pthread_t tid, int dur, int pl, enum OPERATION oper, bool writeToSTDOUT, int fd);

/**
 * @brief Separete log parameters
 * 
 * @param string pointer to the structOp
 * @param i número sequencial do pedido
 * @param pid identificador de sistema do processo
 * @param tid identificador no sistema do thread
 * @param dur duração, em milissegundos, de utilização do Quarto de Banho
 * @param pl nº de lugar que eventualmente lhe será atribuído no Quarto de Banho 
 * @param oper operação que o processo acabou de executar
 */
void receiveLogOperation(structOp *op, int *i, pid_t *pid, pthread_t *tid, int *dur, int *pl , enum OPERATION *oper);