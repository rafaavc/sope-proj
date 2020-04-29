#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "cmdargs.h"
#include "opreg.h"
#include <string.h>
#include <semaphore.h>

#define MAX_STRING_SIZE 512
int nsecs, fd;
char * fifoname;
sem_t sem;

void setArgs(int argc, char ** argv) {
    QArgs args = getCommandLineArgsQ(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    //nthreads = args.nthreads;
    //nplaces = args.nplaces;
}

void *receiveRequest(void * args){
    char *string = malloc(MAX_STRING_SIZE), *stok = malloc(MAX_STRING_SIZE);

    if (read(fd, string, MAX_STRING_SIZE) <= 0) return NULL;
    
    int numbers[6], i = 0;

    stok = strtok(string, ";");

    do {
        numbers[i++] = atoi(stok);
    } while((stok = strtok(NULL, ";")) != NULL);

    for (int i = 0; i < 6; i++){
        printf("%d; ", numbers[i]);
    }
    printf("RECVD\n");

    return NULL;
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    mkfifo(fifoname, 0660);
    fd = open(fifoname, O_RDONLY | O_CREAT, 0644);

    sem_init(&sem, 0, 1);

    int count = 0;
    while(clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs) {
        pthread_t thread;
        pthread_create(&thread, NULL, receiveRequest, NULL);
        count++;
    }

    close(fd);
    unlink(fifoname);
    pthread_exit(0);
}