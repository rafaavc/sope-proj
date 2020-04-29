#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "cmdargs.h"
#include "opreg.h"

#define MAX_STRING_SIZE 512
#define MAX_NUMBER_THREADS 100
int nsecs, fd;
char * fifoname;

void setArgs(int argc, char ** argv) {
    QArgs args = getCommandLineArgsQ(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    //nthreads = args.nthreads;
    //nplaces = args.nplaces;
}

void *receiveRequest(void * args){
    char *string = malloc(MAX_STRING_SIZE);
    read(fd, string, MAX_STRING_SIZE);
    printf("%s", string);
    return NULL;
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    pthread_t threads[MAX_NUMBER_THREADS];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    mkfifo(fifoname, 0660);
    fd = open(fifoname, O_RDONLY | O_CREAT, 0644);

    int count = 0;
    while(clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs && count < MAX_NUMBER_THREADS) {
        pthread_create(&threads[count], NULL, receiveRequest, NULL);
        count++;
    }

    close(fd);
    unlink(fifoname);
    pthread_exit(0);
}