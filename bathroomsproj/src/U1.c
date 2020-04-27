#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STRING_SIZE 512
int nsecs = -1;
char * fifoname;

char * getCommandLineArgs(int argc, char * argv[]) {
    char * fifoname = malloc(MAX_STRING_SIZE);

    char c;
    while((c = getopt(argc, argv, "t:")) != -1){
        switch(c){
            case 't':
                nsecs = atoi(optarg);
                if (nsecs <= 0) {
                    printf("nsecs can't be negative nor 0\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case '?':
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // gets fifoname
    bool found = false;
    int i = 1;
    while(i < argc && argv[i] != NULL){
        if (strcmp(argv[i], "-t") == 0) {
            // when it's -t the value may be separated by space
            i++;
        } else if (argv[i][0] != '-'){
            fifoname = argv[i];
            found = true;
            break;
        }
        i++;
    }

    if (!found) {
        printf("fifoname is required\n");
        exit(EXIT_FAILURE);
    }

    return fifoname;
}

int main(int argc, char ** argv) {
    fifoname = getCommandLineArgs(argc, argv);
    if (nsecs != -1) printf("nsecs: %d\n", nsecs);
    printf("fifoname: %s\n", fifoname);

    exit(EXIT_SUCCESS);
}

