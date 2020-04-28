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
 *  Logs operation to stdout
 */
void logOperation();
