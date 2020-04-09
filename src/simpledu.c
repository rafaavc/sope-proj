#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>
#include "logger.h"


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

int block_size = -1, max_depth = __INT_MAX__;

/*count_links is true because the specifications says "A ferramenta simpledu, como 
exemplificado, deve disponibilizar informação relativa à utilização de
disco por parte de ficheiros e diretórios, considerando sempre que a opção 
-l (ou --count-links) está ativa."*/

bool all = false, bytes = false, count_links = true, dereference = false, separate_dirs = false;

char * logFilename;

int childrenPGID = 0;

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
                    perror("Block-size can't be negative");
                    terminateProcess(EXIT_FAILURE);
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
                    perror("Max depth can't be negative");
                    terminateProcess(EXIT_FAILURE);
                }
                //printf("Max depth = %d\n", max_depth);
                break;
            case '?':
                printUsage();
                terminateProcess(EXIT_SUCCESS);
                break;
            default:
                break;
        }
    }

    // gets path
    int i = 1;
    while(i < argc && argv[i] != NULL){
        if (strcmp(argv[i], "-B") == 0 || strcmp(argv[i], "--block-size")== 0 || strcmp(argv[i], "--max-depth") == 0) {
            // when it's -B, --block-size and --max-depth the value may be separated by space
            i++;
        } else if (argv[i][0] != '-'){
            //printf("Path: %s\n", argv[i]);
            path = argv[i];
            break;
        }
        i++;
    }

    if (path[strlen(path)-1] == '/') {
        for (int i = strlen(path)-1; i >= 0; i--) {
            if (path[i] == '/' && path[i-1] == '/') {
                path[i] = '\0';
            } else {
                break;
            }
        }
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
    if (signo == SIGINT && childrenPGID != 0) {
        logEVENT(RECV_SIGNAL, getpid(), "SIGINT");
        killpg(childrenPGID, SIGSTOP);
        char str[MAX_STRING_SIZE];
        sprintf(str, "SIGSTOP %d", childrenPGID);
        logEVENT(SEND_SIGNAL, getpid(), str);
        while(true) {
            char* opt = malloc(MAX_STRING_SIZE);
            printf("\nAre you sure you want to terminate execution? (Y/N) ");
            scanf("%s", opt);
            char optc = opt[0];
            free(opt);

            if (optc == 'Y' || optc == 'y') {
                printf("Terminating execution.\n");
                killpg(childrenPGID, SIGTERM);
                logEVENT(SEND_SIGNAL, getpid(), str);
                terminateProcess(130);
                break;
            } else if (optc == 'N' || optc == 'n') {
                // Send SIGCONT to children
                printf("Resuming execution.\n");
                killpg(childrenPGID, SIGCONT);
                sprintf(str, "SIGCONT %d", childrenPGID);
                logEVENT(SEND_SIGNAL, getpid(), str);
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
        perror("Unable to install signal handler");
        terminateProcess(EXIT_FAILURE);
    }
}

void printInfoLine(double size, char * path) {
    int sizei = ceil(size);
    if (sizei > 9999999)
        printf("%-15d %s\n", sizei, path);
    else 
        printf("%-7d %s\n", sizei, path);
}

double calculateFileSize(struct stat *stat_buf) {
    int fileSize = 0;
    if (bytes) { 
        fileSize = stat_buf->st_size;
        if (block_size == -1){              // by default block_size = -1 porque quando se usa -b sem -B não se divide por 1024
            return fileSize;
        } else {
            return fileSize*1.0/block_size;
        }
    }
    else fileSize = stat_buf->st_blocks * 512;

    if (block_size == -1)
        return fileSize/1024;
    else {
        return fileSize*1.0/block_size;
    }
}


int writePipe(int fd, void * buffer, int bufferSize) {
    logEVENT(SEND_PIPE, getpid(), buffer);
    return write(fd, buffer, bufferSize);
}

int readPipe(int fd, void * buffer, int bufferSize) {
    int ret = read(fd, buffer, bufferSize);
    logEVENT(RECV_PIPE, getpid(), buffer);
    return ret;
}


void checkDirectory(bool masterProcess, char * path, int currentDepth, int outputFD) {
    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char *newpath = malloc(MAX_STRING_SIZE), * info = malloc(MAX_STRING_SIZE);
    char buffer[MAX_STRING_SIZE];
    double currentDirSize = 0.0;
    double fileSize;
    pid_t pid;
    bool jumpDir = false;


    if (masterProcess) {
        if (lstat(path, &stat_buf) != 0) {
            perror(path);
            terminateProcess(EXIT_FAILURE);
        }
        currentDirSize += calculateFileSize(&stat_buf);
        if (S_ISLNK(stat_buf.st_mode) && !dereference) {
            jumpDir = true;
        }
    }

    if (!jumpDir) {
        if ((dirp = opendir(path)) == NULL)
        {
            perror(path);
            terminateProcess(EXIT_FAILURE);
        }

        while ((direntp = readdir( dirp)) != NULL)
        {
            fileSize = 0;

            // In case it refers to parent directory or same directory
            if (strcmp(direntp->d_name, "..") == 0 || strcmp(direntp->d_name, ".") == 0) continue; 
            
            if (path[strlen(path)-1] == '/') {
                sprintf(newpath, "%s%s", path, direntp->d_name);
            } else {
                sprintf(newpath, "%s/%s", path, direntp->d_name);
            }

            if (lstat(newpath, &stat_buf) != 0) {
                perror(newpath);
                terminateProcess(EXIT_FAILURE);
            }
            
            fileSize = calculateFileSize(&stat_buf);

            /* S_ISREG(m) is it a regular file?
            S_ISDIR(m) directory?
            S_ISCHR(m) character device?
            S_ISBLK(m) block device?
            S_ISFIFO(m) FIFO (named pipe)?
            S_ISLNK(m) symbolic link? (Not in POSIX.1-1996.)
            S_ISSOCK(m) socket? (Not in POSIX.1-1996.) */

            if (S_ISDIR(stat_buf.st_mode) || (S_ISLNK(stat_buf.st_mode) && dereference)) {
                int pipefd[2];
                if (pipe(pipefd) == -1) {
                    perror("Error making pipe");
                    terminateProcess(EXIT_FAILURE);
                }

                if ((pid = fork()) > 0) {
                    close(pipefd[WRITE]);
                    int status;
                    waitpid(pid, &status, 0);

                    readPipe(pipefd[READ], buffer, MAX_STRING_SIZE);
                    close(pipefd[READ]);
                    fileSize += atof(buffer); // very important that it is +=
                    
                    if (currentDepth > 0)
                        printInfoLine(ceil(fileSize), newpath);
                    
                    if (!separate_dirs)
                        currentDirSize += fileSize;

                } else if (pid == 0) {
                    close(pipefd[READ]);
                    
                    sprintf(info, "max-depth: %-10d, %s", currentDepth-1, newpath);
                    logEVENT(CREATE, getpid(), info);

                    checkDirectory(false, newpath, currentDepth-1, pipefd[WRITE]);
                    close(pipefd[WRITE]);
                    terminateProcess(EXIT_SUCCESS);
                } else {
                    perror("Error forking");
                    terminateProcess(EXIT_FAILURE);
                }
            } else {
                if (currentDepth > 0 && all) {
                    printInfoLine(fileSize, newpath);
                }
                currentDirSize += fileSize;
            }

            sprintf(info, "%f \t\t %s", fileSize, newpath);
            logEVENT(ENTRY, getpid(), info);

        }
    }

    sprintf(info, "%f", currentDirSize);
    writePipe(outputFD, info, sizeof(info));

    free(info);

    closedir(dirp);
}

int main(int argc, char* argv[]){ 
    setLogFilename();
    openLogFile();

    char * argvs = satos(argv, argc);
    logEVENT(CREATE, getpid(), argvs);
    free(argvs);

    char *path = getCommandLineArgs(argc, argv);
    installSignalHandler();
    

    int pipefd[2];
    pid_t pid;
    char buffer[MAX_STRING_SIZE];
    char * info = malloc(MAX_STRING_SIZE);

    if (pipe(pipefd) == -1) {
        perror("Error making pipe");
        terminateProcess(EXIT_FAILURE);
    }

    if ((pid = fork()) > 0) {
        close(pipefd[WRITE]);
        childrenPGID = pid;
        int status;
        waitpid(pid, &status, 0);

        readPipe(pipefd[READ], buffer, MAX_STRING_SIZE);
        close(pipefd[READ]);
        int dirSize = atoi(buffer);
        
        printInfoLine(dirSize, path);
        sprintf(info, "%d \t\t %s", dirSize, path);
        logEVENT(ENTRY, getpid(), info);
    } else if (pid == 0) {
        close(pipefd[READ]);

        sprintf(info, "max-depth: %-10d, %s", max_depth, path);
        logEVENT(CREATE, getpid(), info);

        setpgid(0, 0);
        checkDirectory(true, path, max_depth, pipefd[WRITE]);
        close(pipefd[WRITE]);
        terminateProcess(EXIT_SUCCESS);
    } else {
        perror("Error forking");
        terminateProcess(EXIT_FAILURE);
    }

    free(info);
    terminateProcess(EXIT_SUCCESS);
}