# BathroomAccess (t2g07)
2º Projeto para a Unidade Curricular SOPE @ FEUP 2019-2020

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
- [x] nthreads - número máximo de threads em simultâneo
- [x] nplaces - número de casas de banho


### Detalhes
- **Threads** - Utilização de *threads* para atendimento dos clientes (que por si também são *threads*).

- **FIFOs** - Utilização de *FIFOs* para comunização entre as threads dos clientes e o servidor, nomedamente: uma *FIFO* pública, para envio do pedido ao servidor e uma *FIFO* privada, única para cada cliente e através da qual o servidor responde ao pedido do cliente.

- **Mutex** - Utilização de uma *mutex* no servidor (q.c) para o acesso e incremento da variável partilhada pelas *threads* do servidor "placesCount" - contador dos locais alocados aos clientes.

- **Condition Variables** - Utilização de duas *condition variables* no servidor (q.c):
  - para tornar a obtenção de uma vaga na casa de banho mais eficiente. Quando uma thread falha na obtenção de lugar, fica "à espera" que a *condition variable* seja assinalada, momento em que ela volta a tentar obter o lugar. Quando uma thread liberta um lugar, a mesma assinala a *condition variable*.
  - para tornar a espera por um número de threads menor que o máximo (para poder criar outra thread).

- **Alarm** - Utilização de *alarm* e de tratamento do sinal *SIGALRM* para exercer o tempo de execução do programa.
