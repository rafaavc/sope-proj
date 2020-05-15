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

int placesCount = 0; bool * bathrooms; int amountOfThreads = 0; // shared vars
pthread_mutex_t bathroomSpotMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadAmountMut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bathroomSpotCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t threadAmountCond = PTHREAD_COND_INITIALIZER;

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
    alarm(nsecs);
}

void exitThread() {
    if (nthreads != -1) {
        pthread_mutex_lock(&threadAmountMut);
        amountOfThreads--;
        pthread_cond_signal(&threadAmountCond);
        pthread_mutex_unlock(&threadAmountMut);
    }
    pthread_exit(NULL);
}

int openClientFIFO(pid_t pid, pthread_t tid, int i, int dur, int pl) {
    int privatefd;
    char * private_fifoname = malloc(MAX_STRING_SIZE);

    sprintf(private_fifoname, "/tmp/%d.%lu", pid, tid);

    if ((privatefd = open(private_fifoname, O_WRONLY)) == -1){
        logOperation(i, getpid(), pthread_self(), dur, pl, GAVUP, true, NOFD);
        exitThread();
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
    pthread_mutex_lock(&bathroomSpotMut);
    bathrooms[spot] = false;
    pthread_cond_signal(&bathroomSpotCond);
    pthread_mutex_unlock(&bathroomSpotMut);
}

void closeBathroom() {
    bathroomOpen = false;
    pthread_cond_broadcast(&bathroomSpotCond);  // liberta as threads que estão "presas" à espera de vaga
}

void sig_handler(int signo) {
    if (signo == SIGALRM) {
        closeBathroom();
    }
}

void waitForThread() {
    if (nthreads == -1) return;
    pthread_mutex_lock(&threadAmountMut);
    while(true) {
        if (amountOfThreads < nthreads) {
            amountOfThreads++;
            break;
        }
        pthread_cond_wait(&threadAmountCond, &threadAmountMut);
    }
    pthread_mutex_unlock(&threadAmountMut);
}

int waitForBathroomSpot(int i, int pl, int privatefd) {
    int spot;
    pthread_mutex_lock(&bathroomSpotMut);
    while(1){
        if (!bathroomOpen){
            pthread_mutex_unlock(&bathroomSpotMut);
            logOperation(i, getpid(), pthread_self(), -1, pl, TLATE, true, privatefd); // sends response to client
            close(privatefd);
            exitThread();
        }
        if ((spot = getBathroomSpot()) != -1) break;
        pthread_cond_wait(&bathroomSpotCond, &bathroomSpotMut);
    }
    if (nplaces != -1) bathrooms[spot] = true;
    pthread_mutex_unlock(&bathroomSpotMut);

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
        exitThread();
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
    exitThread();
    return NULL;
}

int main(int argc, char ** argv) {
    struct sigaction action;

    action.sa_handler = &sig_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    sigaction(SIGALRM, &action, NULL);

    setArgs(argc, argv);
    
    if (mkfifo(fifoname, 0660) == -1) {
        perror("Error creating public fifo");
        pthread_exit(NULL);
    }

    if ((fd = open(fifoname, O_RDONLY | O_NONBLOCK)) == -1) {
        perror("Error opening public fifo");
        pthread_exit(NULL);
    }

    while(true) {
        pthread_t thread;
        structOp * op = malloc(sizeof(structOp));
        int n;
        if ((n = read(fd, op, sizeof(structOp))) > 0) {
            waitForThread();
            pthread_create(&thread, NULL, receiveRequest, op);
        } else {
            free(op);
            if (n == -1) {
                if (errno != EAGAIN) { // EAGAIN happens when the write end has been opened but hasn't written yet (because of O_NONBLOCK)
                    perror("Error reading public fifo");
                    pthread_exit(NULL);
                }  
            } else if (!bathroomOpen) { // n == 0: end of file -> fifo vazio, não aberto para escrita
                break;
            }
        }
    }

    close(fd);
    unlink(fifoname);
    free(bathrooms);
    pthread_exit(EXIT_SUCCESS);
}