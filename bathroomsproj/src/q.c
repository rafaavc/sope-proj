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

int nsecs, fd, nthreads = -1, nplaces = -1;
char * fifoname;
bool bathroomOpen = true;
bool *bathroom;

int placesCount = 0; // shared vars
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int checkEmptyBathroom(){
    for (int i = 0; i < 10; i++){
        if (bathroom[i] == false) return i;
    }
    return -1;
}

void setArgs(int argc, char ** argv) {
    QArgs args = getCommandLineArgsQ(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    nthreads = args.nthreads;
    nplaces = args.nplaces;
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

    int n;
    while(true){
        if (!bathroomOpen){
            logOperation(i, getpid(), pthread_self(), -1, pl, TLATE, true, privatefd);
            close(privatefd);
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&mut);
        if ((n = checkEmptyBathroom()) >= 0) break;
        pthread_mutex_unlock(&mut);

    }

    bathroom[n] = true;
    logOperation(i, getpid(), pthread_self(), dur, n, ENTER, true, privatefd);
    placesCount++;

    pthread_mutex_unlock(&mut);

    usleep(dur*1000);
    bathroom[n] = false;
    logOperation(i, getpid(), pthread_self(), dur, pl, TIMUP, true, NOFD);

    close(privatefd);
    pthread_exit(NULL);
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    if (nplaces == -1){
        printf("nplaces is required\n");
        exit(EXIT_FAILURE);
    }
    bathroom = calloc(nplaces, sizeof(bool));
    
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