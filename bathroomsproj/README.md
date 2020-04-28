# Simpledu (t2g07)
Projeto para a Unidade Curricular SOPE @ FEUP 2019-2020


### Funcionalidades
- [x] du (-l)
- [x] -a/--all
- [X] -b/--bytes
- [X] -B/--block-size
- [x] -L/--dereference
- [x] -S/--separate-dirs
- [x] --max-depth

Nota: se a log file estiver dentro do diretório que está a ser analisado os resultados do simpledu quando comparados com o du serao provavelmente diferentes (devido ao ficheiro de log).


### Detalhes
**Sinais** - É tratado o sinal SIGINT e é recebido pelo pai. Todos os processos-filho têm o mesmo grupo que é diferente do grupo do processo-pai e ao receber o sinal, este envia para o grupo dos processos-filho o sinal SIGSTOP, se o utilizador quiser cancelar o programa envia SIGTERM, senão envia SIGCONT.

**Pipes** - Os subdiretórios de cada diretório (o que está a ser analisado) escrevem para um pipe a struct pipeInfo que contem o seu tamanho e se era um diretório ou nao (no caso de symbolic links para ficheiros), o processo que está encarregue do diretório recebe estas informações, adiciona o tamanho do seu subdiretório ao seu próprio tamanho (se a opção separate-dirs não estiver ativa) e, dependendo dos argumentos do comando, escreve-as para o output. Este diretório escreve então a sua própria struct pipeInfo outro pipe (resultante da soma do tamanho dos seus conteúdos).

**Forks** - A utilização de forks é feita para ler o conteúdo das subpastas com recursão para a função checkDirectory, o processo-pai fica na pasta corrente e o processo-filho segue para a subpasta.
