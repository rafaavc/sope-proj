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
#include <stdbool.h>

#define MAX_STRING_SIZE 512
int nsecs, fd;
char * fifoname;
sem_t empty;
bool bathroomOpen = true;

void setArgs(int argc, char ** argv) {
    QArgs args = getCommandLineArgsQ(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    //nthreads = args.nthreads;
    //nplaces = args.nplaces;
}


void *receiveRequest(void * args){
    char *private_fifoname = malloc(MAX_STRING_SIZE);
    char *string = (char *)args;
    int i, pid, dur, pl, privatefd;
    long t, tid;
    enum OPERATION oper;

    
    int numbers[6];

    receiveLogOperation(&string[0], &t, &i, &pid, &tid, &dur, &pl, &oper);
    oper = RECVD;
    logOperation(i, getpid(), pthread_self(), dur, pl, oper, STDOUT_FILENO);

    sprintf(private_fifoname, "/tmp/%d.%ld", pid, tid);

    if ((privatefd = open(private_fifoname, O_WRONLY)) <= 0){
        logOperation(i, getpid(), pthread_self(), dur, pl, GAVUP);
        return NULL;
    }


    if (!bathroomOpen){
        string = logOperation(i, getpid(), pthread_self(), dur, pl, TLATE, STDOUT_FILENO);
        write(privatefd, string, strlen(string));
        return NULL;
    }

    static int count = 0;
    string = logOperation(i, getpid(), pthread_self(), dur, count, ENTER, STDOUT_FILENO);
    count++;
    write(privatefd, string, strlen(string));

    usleep(dur);

    string = logOperation(i, getpid(), pthread_self(), dur, pl, TIMUP, STDOUT_FILENO);

    return NULL;
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    mkfifo(fifoname, 0660);
    fd = open(fifoname, O_RDONLY, 0644);

    sem_init(&empty, 0, 1);

    int count = 0;
    while(clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs) {
        pthread_t thread;
        char *string = malloc(MAX_STRING_SIZE);
        if (read(fd, string, MAX_STRING_SIZE) > 0){
            pthread_create(&thread, NULL, receiveRequest, (int *) &string[0]);
            count++;
        }
    }
    bathroomOpen = false;

    close(fd);
    unlink(fifoname);
    pthread_exit(EXIT_SUCCESS);
}