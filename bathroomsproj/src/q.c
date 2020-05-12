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

int nsecs, fd, nthreads, nplaces;
char * fifoname;
bool bathroomOpen = true;

int placesCount = 0; bool * bathrooms; // shared vars
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void setArgs(int argc, char ** argv) {
    QArgs args = getCommandLineArgsQ(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    nthreads = args.nthreads;
    nplaces = args.nplaces;
    if (nplaces != -1) {
        bathrooms = malloc(sizeof(bool)*nplaces);
        for (int i = 0; i < nplaces; i++) bathrooms[i] = false;
    }
}

int openClientFIFO(pid_t pid, pthread_t tid, int i, int dur, int pl) {
    int privatefd;
    char * private_fifoname = malloc(MAX_STRING_SIZE);

    sprintf(private_fifoname, "/tmp/%d.%lu", pid, tid);

    if ((privatefd = open(private_fifoname, O_WRONLY)) == -1){
        logOperation(i, getpid(), pthread_self(), dur, pl, GAVUP, true, NOFD);
        pthread_exit(NULL);
    }

    free(private_fifoname);
    return privatefd;
}

int getBathroomSpot() {
    if (nplaces == -1) return placesCount++;
    else {
        for (int i = 0; i < nplaces; i++){
            if (bathrooms[i] == false) return i;
        }
    }
    return -1;
}

void freeSpot(int spot) {
    pthread_mutex_lock(&mut);
    bathrooms[spot] = false;
    pthread_mutex_unlock(&mut);
}

int waitForBathroomSpot(int i, int pl, int privatefd) {
    int spot;
    while(1){
        if (!bathroomOpen){
            logOperation(i, getpid(), pthread_self(), -1, pl, TLATE, true, privatefd); // sends response to client
            close(privatefd);
            pthread_exit(NULL);
        }
        pthread_mutex_lock(&mut);
        if ((spot = getBathroomSpot()) != -1) break;
        pthread_mutex_unlock(&mut);
    }
    if (nplaces != -1) bathrooms[spot] = true;
    pthread_mutex_unlock(&mut);

    return spot;
}

void * receiveRequest(void * args){
    structOp * op = (structOp *) args;
    int i, dur, pl, privatefd;
    pid_t pid;
    pthread_t tid;
    enum OPERATION oper;

    receiveLogOperation(op, &i, &pid, &tid, &dur, &pl, &oper);
    free(op);

    if (oper != IWANT) {
        write(STDERR_FILENO, "Unknown operation\n", 18);
        pthread_exit(NULL);
    }

    logOperation(i, getpid(), pthread_self(), dur, pl, RECVD, true, NOFD); // logs reception of operation

    privatefd = openClientFIFO(pid, tid, i, dur, pl); // opens the client fifo to send the response to

    int spot;
    spot = waitForBathroomSpot(i, pl, privatefd); // waits for the bathroom spot
    
    logOperation(i, getpid(), pthread_self(), dur, spot, ENTER, true, privatefd); // sends response to client

    usleep(dur*1000); // Duration of the bathroom usage

    logOperation(i, getpid(), pthread_self(), dur, spot, TIMUP, true, NOFD); // logs the end of the bathroom usage

    if (nplaces != -1) freeSpot(spot); // in case of limited spots, frees the spot

    close(privatefd);
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

    free(bathrooms);
    close(fd);
    unlink(fifoname);
    pthread_exit(EXIT_SUCCESS);
}