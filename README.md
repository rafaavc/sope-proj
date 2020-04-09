# disk-space-check
Project for the Operating Systems class @ FEUP


### Funcionalidades
- [x] du (-l)
- [x] -a/--all
- [X] -b/--bytes
- [X] -B/--block-size
- [x] -L/--dereference   (loop de symbolic links!)
- [x] -S/--separate-dirs
- [x] --max-depth

Nota: se a log file estiver dentro do diretório que está a ser analisado os resultados do simpledu quando comparados com o du serao provavelmente diferentes (devido ao ficheiro de log).


### Detalhes
**Sinais** - É tratado o sinal SIGINT e é recebido pelo pai. Todos os processos-filho têm o mesmo grupo que é diferente do grupo do processo-pai e ao receber o sinal, este envia para o grupo dos processos-filho o sinal SIGSTOP, se o utilizador quiser cancelar o programa envia SIGTERM, senão envia SIGCONT.

**Pipes** - Os diretórios de cada pasta escrevem para um pipe o seu tamanho, a pasta recebe estas informações e dependendo do comando escreve-as e/ou adiciona ao seu tamanho e escreve para outro pipe o seu tamanho (resultante da soma do tamanho dos seus conteúdos).

**Forks** - A utilização de forks é feita para ler o conteúdo das subpastas com recursão para a função checkDirectory, o processo-pai fica na pasta corrente e o processo-filho segue para a subpasta.
