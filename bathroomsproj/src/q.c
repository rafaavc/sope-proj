#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "cmdargs.h"
#include "opreg.h"

#define MAX_STRING_SIZE 512
int nsecs;
char * fifoname;

void setArgs(int argc, char ** argv) {
    QArgs args = getCommandLineArgsQ(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    //nthreads = args.nthreads;
    //nplaces = args.nplaces;
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    mkfifo(fifoname, 0660);
    int fd = open(fifoname, O_RDONLY | O_NONBLOCK);

    int count = 0;
    while(clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs) {
        printf("receiving request %d\n", count);
        count++;
    }

    close(fd);
    unlink(fifoname);
    exit(EXIT_SUCCESS);
}