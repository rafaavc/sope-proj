#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "cmdargs.h"

#define MAX_STRING_SIZE 512
int nsecs;
char * fifoname;
bool bathroomOpen = true;


void setArgs(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    srand(time(0));
    int count = 0;
    while(bathroomOpen && (clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs)) {
        printf("start thread %d\n", count);
        unsigned msInterval = 80 + (((float)rand()/RAND_MAX)*80);
        usleep(msInterval*1000); // sleeps a random number of milliseconds (from 80 to 160)
        count++;
    }

    exit(EXIT_SUCCESS);
}

