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


#define MAX_PATH_SIZE 512
#define MAX_DEPTH 'm'


int block_size = 1024, max_depth = __INT_MAX__;

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
    //bool para opções variadas
    bool all = false, bytes = false, count_links = false, dereference = false, separate_dirs = false;


    //lê todas as opções (-char) que forem passadas no argv
    while((c = getopt_long(argc, argv, "abB:lLS", long_options, NULL)) != -1){
        switch(c){
            case 0:     //long option
                break;
            case 'a':
                all = 0;
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
        }

    }
    exit(0);
}