#include <stdlib.h>
#include <stdio.h>
#include "cmdargs.h"
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STRING_SIZE 512

char * getFifoname(int argc, char * argv[]) {
    char * fifoname = malloc(MAX_STRING_SIZE);

    // gets fifoname
    bool found = false;
    int i = 1;
    while(i < argc && argv[i] != NULL){
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "-l") == 0) {
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

QArgs getCommandLineArgsQ(int argc, char * argv[]) {
    QArgs res;

    res.nsecs = -1;
    res.nplaces = -1;
    res.nthreads = -1;

    char c;
    while((c = getopt(argc, argv, "t:l:n:")) != -1){
        switch(c){
            case 't':
                res.nsecs = atoi(optarg);
                if (res.nsecs <= 0) {
                    printf("nsecs can't be negative nor 0\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                // for phase 2
                break;
            case 'n':
                // for phase 2
                break;
            case '?':
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    res.fifoname = getFifoname(argc, argv);

    return res;
}

UArgs getCommandLineArgsU(int argc, char * argv[]) {
    UArgs res;

    res.nsecs = -1;

    char c;
    while((c = getopt(argc, argv, "t:")) != -1){
        switch(c){
            case 't':
                res.nsecs = atoi(optarg);
                if (res.nsecs <= 0) {
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

    res.fifoname = getFifoname(argc, argv);

    return res;
}

