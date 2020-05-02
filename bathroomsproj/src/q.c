#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "cmdargs.h"
#include "opreg.h"
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_STRING_SIZE 512

int nsecs, fd;
char * fifoname;
bool bathroomOpen = true;

int placesCount = 0; // shared vars
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void setArgs(int argc, char ** argv) {
    QArgs args = getCommandLineArgsQ(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    //nthreads = args.nthreads;
    //nplaces = args.nplaces;
}

void *receiveRequest(void * args){
    char *private_fifoname = malloc(MAX_STRING_SIZE);
    structOp *op = (structOp *) args;
    int i, dur, pl, privatefd;
    pid_t pid;
    pthread_t tid;
    enum OPERATION oper;

    receiveLogOperation(op, &i, &pid, &tid, &dur, &pl, &oper);
    free(op);

    if (oper == IWANT) {
        logOperation(i, getpid(), pthread_self(), dur, pl, RECVD, true, NOFD);

        sprintf(private_fifoname, "/tmp/%d.%lu", pid, tid);
    }

    //usleep((500 + rand()%1000)*1000); // Simulating waiting for spot

    if ((privatefd = open(private_fifoname, O_WRONLY)) == -1){
        logOperation(i, getpid(), pthread_self(), dur, pl, GAVUP, true, NOFD);
        pthread_exit(NULL);
    }
    free(private_fifoname);

    if (!bathroomOpen){
        sleep(1);
        logOperation(i, getpid(), pthread_self(), -1, pl, TLATE, true, privatefd);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&mut);
    logOperation(i, getpid(), pthread_self(), dur, placesCount, ENTER, true, privatefd);
    placesCount++;
    pthread_mutex_unlock(&mut);

    usleep(dur*1000);

    logOperation(i, getpid(), pthread_self(), dur, pl, TIMUP, true, NOFD);

    pthread_exit(NULL);
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    if (mkfifo(fifoname, 0660) == -1) {
        perror("Error creating public fifo");
        pthread_exit(NULL);
    }

    if ((fd = open(fifoname, O_RDONLY | O_NONBLOCK)) == -1) {
        perror("Error opening public fifo");
        pthread_exit(NULL);
    }

    while(clock_gettime(CLOCK_MONOTONIC_RAW, &end), (end.tv_sec + (end.tv_nsec/(1000000000.))) - (start.tv_sec + (start.tv_nsec/(1000000000.))) < nsecs) {
        pthread_t thread;
        structOp * op = malloc(sizeof(structOp));
        int n;
        if ((n = read(fd, op, sizeof(structOp))) > 0) {
            pthread_create(&thread, NULL, receiveRequest, op);
        } else if (n == -1) {
            if (errno != EAGAIN) { // EAGAIN happens when the write end hasn't been opened yet (because of O_NONBLOCK)
                perror("Error reading public fifo");
                pthread_exit(NULL);
            }
        } else {
            free(op);
        }
    }
    bathroomOpen = false;

    close(fd);
    unlink(fifoname);
    pthread_exit(EXIT_SUCCESS);
}