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

int block_size = -1, max_depth = __INT_MAX__;

/*count_links is true because the specifications says "A ferramenta simpledu, como 
exemplificado, deve disponibilizar informação relativa à utilização de
disco por parte de ficheiros e diretórios, considerando sempre que a opção 
-l (ou --count-links) está ativa."*/

bool all = false, bytes = false, count_links = true, dereference = false, separate_dirs = false;

char * logFilename;

int childrenPGID = 0;


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
            // when it's -B, --block-size and --max-depth the value may be separated by space
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
    if (signo == SIGINT && childrenPGID != 0) {
        char* opt = malloc(MAX_STRING_SIZE);

        killpg(childrenPGID, SIGSTOP);
        while(true) {
            printf("Are you sure you want to terminate execution? (Y/N) ");
            scanf("%s", opt);
            if (opt[0] == 'Y' || opt[0] == 'y') {
                printf("Terminating execution.\n");
                killpg(childrenPGID, SIGTERM);
                exit(7);
                break;
            } else if (opt[0] == 'N' || opt[0] == 'n') {
                // Send SIGCONT to children
                printf("Resuming execution.\n");
                killpg(childrenPGID, SIGCONT);
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

void printInfoLine(int size, char * path) {
    printf("%-7d %s\n", size, path);
}

int calculateFileSize(struct stat *stat_buf) {
    int fileSize = 0;
    if (bytes) { 
        fileSize = stat_buf->st_size;
        if (block_size == -1){              // by default block_size = -1 porque quando se usa -b sem -B não se divide por 1024
            return fileSize;
        } else {
            return fileSize/block_size;
        }
    }
    else fileSize = ceil(stat_buf->st_blocks * 512);

    if (block_size == -1)
        return ceil(fileSize/1024);
    else 
        return ceil(fileSize/block_size);
}


void checkDirectory(bool masterProcess, char * path, int currentDepth, int outputFD) {
    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char *newpath = malloc(MAX_STRING_SIZE);
    char buffer[MAX_STRING_SIZE];
    int currentDirSize = 0;
    int fileSize;
    pid_t pid;


    if (masterProcess) {
        if (lstat(path, &stat_buf) != 0) {
            perror(path);
            exit(3);
        }
        currentDirSize += calculateFileSize(&stat_buf);
    }

    if ((dirp = opendir(path)) == NULL)
    {
        perror(path);
        exit(2);
    }

    /* struct dirent {
        ino_t          d_ino;        Inode number 
        off_t          d_off;        Not an offset; see below 
        unsigned short d_reclen;     Length of this record 
        unsigned char  d_type;       Type of file; not supported by all filesystem types 
        char           d_name[256];  Null-terminated filename 
    }; */

    while ((direntp = readdir( dirp)) != NULL)
    {
        fileSize = 0;

        // In case it refers to parent directory or same directory
        if (strcmp(direntp->d_name, "..") == 0 || strcmp(direntp->d_name, ".") == 0) continue; 
        
        sprintf(newpath, "%s/%s", path, direntp->d_name);

        /* struct stat {
            dev_t     st_dev;      ID of device containing file 
            ino_t     st_ino;      inode number 
            mode_t    st_mode;     protection 
            nlink_t   st_nlink;    number of hard links 
            uid_t     st_uid;      user ID of owner 
            gid_t     st_gid;      group ID of owner 
            dev_t     st_rdev;     device ID (if special file) 
            off_t     st_size;     total size, in bytes 
            blksize_t st_blksize;  blocksize for file system I/O 
            blkcnt_t  st_blocks;   number of 512B blocks allocated 
            time_t    st_atime;    time of last access 
            time_t    st_mtime;    time of last modification 
            time_t    st_ctime;    time of last status change 
        };
        returns information about the file in the struct stat format */
        if (lstat(newpath, &stat_buf) != 0) {
            perror(path);
            exit(3);
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
                printf("ERROR PIPEING\n");
                exit(6);
            }

            if ((pid = fork()) > 0) {
                close(pipefd[WRITE]);
                int status;
                waitpid(pid, &status, 0);

                read(pipefd[READ], &buffer, MAX_STRING_SIZE);
                close(pipefd[READ]);
                fileSize += atoi(buffer); // very important that it is +=
                
                if (currentDepth > 0)
                    printInfoLine(fileSize, newpath);
                
                if (!separate_dirs)
                    currentDirSize += fileSize;

            } else if (pid == 0) {
                close(pipefd[READ]);
                checkDirectory(false, newpath, currentDepth-1, pipefd[WRITE]);
                close(pipefd[WRITE]);
                exit(0);
            } else {
                printf("Error forking\n");
                exit(5);
            }
        } else {
            if (currentDepth > 0 && all) {
                printInfoLine(fileSize, newpath);
            }
            currentDirSize += fileSize;
        }
    }


    char *b = malloc(MAX_STRING_SIZE);
    sprintf(b, "%d", currentDirSize);
    write(outputFD, b, sizeof(b));

    closedir(dirp);
}

// TODO check return codes
int main(int argc, char* argv[]){
    char *path = getCommandLineArgs(argc, argv); 
    setLogFilename(); // i suggest that we create the logger functions in a separate file
    installSignalHandler();
    /*
    Doubt: Do we need to log all signals or only SIGINT's?
    */
    int pipefd[2];
    pid_t pid;
    char buffer[MAX_STRING_SIZE];

    if (pipe(pipefd) == -1) {
        printf("ERROR PIPEING\n");
        exit(6);
    }

    if ((pid = fork()) > 0) {
        close(pipefd[WRITE]);
        childrenPGID = pid;
        int status;
        waitpid(pid, &status, 0);

        read(pipefd[READ], &buffer, MAX_STRING_SIZE);
        close(pipefd[READ]);
        int dirSize = atoi(buffer);

        printInfoLine(dirSize, path);
    } else if (pid == 0) {
        close(pipefd[READ]);
        setpgid(0, 0);
        checkDirectory(true, path, max_depth, pipefd[WRITE]);
        close(pipefd[WRITE]);
        exit(0);
    } else {
        printf("Error forking\n");
        exit(5);
    }

    exit(0);
}