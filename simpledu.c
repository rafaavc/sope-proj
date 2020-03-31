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


#define MAX_STRING_SIZE 512
#define MAX_DEPTH 'm'


int block_size = 1024, max_depth = __INT_MAX__;
bool all = false, bytes = false, count_links = false, dereference = false, separate_dirs = false;

bool check_stat(struct stat stat_buf){
    if (S_ISLNK(stat_buf.st_mode)){
        if (all || dereference) return true;
    } 
    else if(S_ISREG(stat_buf.st_mode)){
        if (all) return true;
    }
    return false;
}

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
    if (separate_dirs) cmd[i++] = "-s";
    sprintf(tmp, "--max-depth=%d", max_depth - 1);
    cmd[i++] = tmp;
    cmd[i++] = path;
    cmd[i] = NULL;
}


//struct para opções longas funcionarem com getopt_long
struct option const long_options[] = {
    {"all", no_argument, NULL, 'a'},
    {"bytes", no_argument, NULL, 'b'},
    {"block-size", required_argument, 0, 'B'},
    {"count-links", no_argument, NULL, 'l'},
    {"dereference", no_argument, NULL, 'L'},
    {"separate-dirs", no_argument, NULL, 's'},
    {"max-depth", required_argument, 0, MAX_DEPTH}

};

int main(int argc, char* argv[]){

    char c;
    char *path = malloc(MAX_STRING_SIZE);
    strcpy(path,".");
    //bool para opções variadas


    //lê todas as opções (-char) que forem passadas no argv
    while((c = getopt_long(argc, argv, "abB:lLS", long_options, NULL)) != -1){
        switch(c){
            case 0:     //long option
                break;
            case 'a':
                all = true;
                break;
            case 'b':
                bytes = true;
                break;
            case 'B':
                block_size = atoi(optarg);
                if (block_size < 0){
                    printf("Block-size can't be negative.\n");
                    exit(2);
                }
                break;
            case 'l':
                count_links = true;
                break;
            case 'L':
                dereference = true;
                break;
            case 'S':
                separate_dirs = true;
                break;
            case MAX_DEPTH:
                max_depth = atoi(optarg);
                if (max_depth < 0){
                    printf("Max depth can't be negative.\n");
                    exit(2);
                }
                break;
            default:
                break;
        }

    }

    int i = 1;
    while(i < argc && argv[i] != NULL){
        if (argv[i][0] != '-'){
            path = argv[i];
            break;
        }
        i++;
    }

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

        if (check_stat(stat_buf)){
            // Tamanho do bloco = 512, para ter tamanho em 4096, nº blocos/8
            int file_space = stat_buf.st_blksize * stat_buf.st_blocks/8;
            printf("%-7d %s\n", (int)ceil(file_space/block_size), newpath);

        } else if (S_ISDIR(stat_buf.st_mode)){
            if(strcmp(direntp->d_name, ".") != 0){
                if (fork() > 0){
                    int file_space = stat_buf.st_blksize * stat_buf.st_blocks/8;
                    printf("%-7d %s\n", (int)ceil(file_space/block_size), newpath);
                    wait(NULL);
                } else if (max_depth > 1){
                    char ** command;
                    prepare_command(command, newpath);
                    closedir(dirp);
                    execv(command[0], command);
                } else {
                    exit(0);
                }
            }


        }
    }
    closedir(dirp);
    exit(0);
}