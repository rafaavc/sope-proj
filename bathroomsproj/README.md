# BathroomAccess (t2g07)
2º Projeto para a Unidade Curricular SOPE @ FEUP 2019-2020

Uso:
make
./Q1 -t nsecs fifoname, sendo nsecs o número de segundos que a casa de banho está aberta
./U1 -t nsecs fifoname, sendo nsecs o número de segundos durante os quais se geram clientes

sendo fifoname o nome da fifo pública utilizada 


### Funcionalidades
- [x] IWANT - cliente faz pedido inicial
- [x] RECVD - servidor acusa receção de pedido
- [x] ENTER - servidor diz que aceitou pedido
- [x] IAMIN - cliente acusa a utilização do Quarto de Banho
- [x] TIMUP - servidor diz que terminou o tempo de utilização
- [x] 2LATE - servidor rejeita pedido por Quarto de Banho já ter encerrado
- [x] CLOSD - cliente acusa informação de que o Quarto de Banho está fechado
- [x] FAILD - cliente já não consegue receber resposta do servidor
- [x] GAVUP - servidor já não consegue responder a pedido porque FIFO privado do cliente fechou

Nota: nesta primeira fase é um pouco difícil testar as operações 2LATE e CLOSD. Para facilitar é possível descomentar a linha 48 do ficheiro 'q.c', que simula um tempo de espera entre 0.5 e 1.5 segundos por cliente.


### Detalhes
- **Threads** - Utilização de threads para atendimento dos clientes (que por si também são threads).

- **FIFOs** - Utilização de FIFOs para comunização entre as threads dos clientes e o servidor, nomedamente: uma fifo pública, para envio do pedido ao servidor e uma fifo privada, única para cada cliente e através da qual o servidor responde ao pedido do cliente.

- **Mutex** - Utilização de uma mutex no servidor (q.c) para o acesso e incremento da variável partilhada pelas threads do servidor "placesCount" - contador dos locais alocados aos clientes.

- **Condition Variable** - Utilização de uma condition variable no servidor (q.c) para tornar a obtenção de uma vaga na casa de banho mais eficiente. Quando uma thread falha na obtenção de lugar, fica "à espera" que a *condition variable* seja assinalada, momento em que ela volta a tentar obter o lugar. Quando uma thread liberta um lugar, a mesma assinala a *condition variable*.
