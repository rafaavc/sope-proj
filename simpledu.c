#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>


#define MAX_STRING_SIZE 512
#define MAX_DEPTH 'm'
#define WRITE 1
#define READ 0


/*
From the specifications:

"Na ferramenta proposta para este trabalho, deve tentar-se reproduzir a informação
apresentada pelo comando du correntemente instalado. Por omissão, o comando du:
    - apresenta o espaço ocupado em número de blocos de 1024 bytes;
    - apenas lista diretórios;
    - não segue links simbólicos;
    - contabiliza uma única vez cada ficheiro;
    - apresenta de forma cumulativa o tamanho de subdiretórios e ficheiros incluídos;
    - não restringe os níveis de profundidade na estrutura de diretórios.""
*/

int block_size = 1024, max_depth = __INT_MAX__;

/*count_links is true because the specifications says "A ferramenta simpledu, como 
exemplificado, deve disponibilizar informação relativa à utilização de
disco por parte de ficheiros e diretórios, considerando sempre que a opção 
-l (ou --count-links) está ativa."*/

bool all = false, bytes = false, count_links = true, dereference = false, separate_dirs = false;

char * logFilename;


void prepare_command(char **cmd, char* path){
    char * tmp = malloc(MAX_STRING_SIZE);
    int i = 0;

    cmd[i++] = "simpledu";
    if (all) cmd[i++] = "-a";
    if (bytes) cmd[i++] = "-b";
    sprintf(tmp, "-B=%d", block_size);
    cmd[i++] = tmp;
    if (count_links) cmd[i++] = "-l";
    if (dereference) cmd[i++] = "-L";
    if (separate_dirs) cmd[i++] = "-S";
    if (max_depth > 0) sprintf(tmp, "--max-depth=%d", max_depth - 1);
    else sprintf(tmp, "--max-depth=%d", max_depth);

    cmd[i++] = tmp;
    cmd[i++] = path;
    cmd[i] = NULL;
}

void printUsage() {
    printf("\nUsage:\n\nsimpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\nsimpledu --count-links [path] [--all] [--bytes] [--block-size size] [--dereference] [--separate-dirs] [--max-depth=N]\n");
}

char * getCommandLineArgs(int argc, char * argv[]) {
    char * path = malloc(MAX_STRING_SIZE);
    strcpy(path,".");

    //struct para opções longas funcionarem com getopt_long
    struct option const long_options[] = {
        {"all", no_argument, NULL, 'a'},
        {"bytes", no_argument, NULL, 'b'},
        {"block-size", required_argument, 0, 'B'},
        {"count-links", no_argument, NULL, 'l'},
        {"dereference", no_argument, NULL, 'L'},
        {"separate-dirs", no_argument, NULL, 'S'},
        {"max-depth", required_argument, NULL, MAX_DEPTH}
    };

    //lê todas as opções que forem passadas no argv (exceto o path)
    char c;
    while((c = getopt_long(argc, argv, "abB:lLS", long_options, NULL)) != -1){
        switch(c){
            case 0:     //long option
                printf("ZERO\n");
                break;
            case 'a':
                all = true;
                ///printf("all\n");
                break;
            case 'b':
                bytes = true;
                //printf("bytes\n");
                break;
            case 'B':
                block_size = atoi(optarg);
                if (block_size < 0){
                    printf("Block-size can't be negative.\n");
                    exit(2);
                }
                //printf("Block size: %s\n", optarg);
                break;
            case 'l':
                // count_links is assumed to be always activated
                // printf("count_links\n");
                break;
            case 'L':
                dereference = true;
                //printf("dereference\n");
                break;
            case 'S':
                separate_dirs = true;
                //printf("separate_dirs\n");
                break;
            case MAX_DEPTH:
                max_depth = atoi(optarg);
                if (max_depth < 0){
                    printf("Max depth can't be negative.\n");
                    exit(2);
                }
                //printf("Max depth = %d\n", max_depth);
                break;
            case '?':
                printUsage();
                exit(1);
                break;
            default:
                break;
        }
    }

    // gets path
    int i = 1;
    while(i < argc && argv[i] != NULL){
        if (strcmp(argv[i], "-B") == 0 || strcmp(argv[i], "--block-size")== 0 || strcmp(argv[i], "--max-depth") == 0) {
            // when it's -B, --block-size and --max-depth the value will be separated by space
            i++;
        } else if (argv[i][0] != '-'){
            //printf("Path: %s\n", argv[i]);
            path = argv[i];
            break;
        }
        i++;
    }
    return path;
}

void setLogFilename() {
    logFilename = getenv("LOG_FILENAME");
    if (logFilename == NULL) {
        logFilename = "./log.txt";
    }
}

void signalHandler(int signo) {
    if (signo == SIGINT) {
        char* opt;

        // Send SIGSTOP to children (use int killpg(int pgrp, int sig) to send the children to the pgroup created)
        while(true) {
            printf("Are you sure you want to terminate execution? (Y/N) ");
            scanf("%s", opt);
            if (opt[0] == 'Y' || opt[0] == 'y') {
                // Send SIGTERM to children
                printf("Terminating execution.\n");
                break;
            } else if (opt[0] == 'N' || opt[0] == 'n') {
                // Send SIGCONT to children
                printf("Resuming execution.\n");
                break;
            }
        }
    }
}

void installSignalHandler() {
    struct sigaction action;

    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) == -1) {
        printf("Unable to install signal handler.\n");
        exit(4);
    }
}

int main(int argc, char* argv[]){
    char *path = getCommandLineArgs(argc, argv); 
    setLogFilename(); // i suggest that we create the logger functions in a separate file
    installSignalHandler();

    int folder_size = 0;

    

    /*
    All children need to be changed into another process group so that SIGINT is sent only to the main process
    Doubt: Do we need to log all signals or only SIGINT's?
    */

    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char *newpath = malloc(MAX_STRING_SIZE);

    if ((dirp = opendir(path)) == NULL)
    {
        perror(path);
        exit(2);
    }
    while ((direntp = readdir( dirp)) != NULL)
    {
        sprintf(newpath, "%s/%s", path, direntp->d_name);

        if (lstat(newpath, &stat_buf) != 0) {
            perror(path);
            exit(3);
        }
        if (strcmp(direntp->d_name, "..") == 0) continue; 

        int file_space = stat_buf.st_blksize * stat_buf.st_blocks/8;
        
        
        if (S_ISREG(stat_buf.st_mode)){
            if (all && max_depth != 0){
                char temp[100];
                sprintf(temp, "%-7d %s\n", (int)ceil(file_space/block_size), newpath);
                write(STDOUT_FILENO, &temp, strlen(temp) + 1);
            } 
            if (separate_dirs){
                folder_size += file_space;
            }
        } else if (S_ISDIR(stat_buf.st_mode)){
            if(strcmp(direntp->d_name, ".") != 0){
                if (fork() > 0){
                    wait(NULL);
                } else {
                    //Mantém sem o NULL porque senão dá erro na prepare_command e não faz em depth
                    char *command[100];
                    prepare_command(command, newpath);
                    closedir(dirp);
                    execv(command[0], command);
                }
            } else {
                folder_size += file_space;
            }
        } else if (S_ISLNK(stat_buf.st_mode)){
            if ((all && dereference) || dereference){
                if (fork() > 0){
                    wait(NULL);
                } else {
                    //Mantém sem o NULL porque senão dá erro na prepare_command e não faz em depth
                    char *command[100];
                    prepare_command(command, newpath);
                    closedir(dirp);
                    execv(command[0], command);
                }
            } else if (all){
                char temp[100];
                sprintf(temp, "%-7d %s\n", (int)ceil(file_space/block_size), newpath);
                write(STDOUT_FILENO, &temp, strlen(temp) + 1);
                folder_size += file_space;
            }
        }
    }
    if (max_depth > 0) printf("%-7d %s\n", folder_size/block_size, path);

    closedir(dirp);
    exit(0);
}