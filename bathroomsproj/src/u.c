#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "cmdargs.h"
#include "opreg.h"
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#define MAX_STRING_SIZE 512
int nsecs;
char * fifoname;
bool bathroomOpen = true;


void setArgs(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
}

void waitResponse(int privatefd){
    structOp *op = malloc(sizeof(structOp));
    int i, dur, pl;
    pid_t pid;
    pthread_t tid;
    enum OPERATION oper;

    int n;

    while((n = read(privatefd, op, sizeof(structOp))) <= 0 /*&& timeElapsed < nsecs*/) {
        if (n == -1) {
            if (errno != EAGAIN) {
                perror("Error reading from private fifo");
                return;
            }
        }
    }

    receiveLogOperation(op, &i, &pid, &tid, &dur, &pl, &oper);
    
    switch(oper) {
        case ENTER:
            logOperation(i, getpid(), pthread_self(), dur, pl, IAMIN, true, NOFD);
            break;
        case TLATE:
            logOperation(i, getpid(), pthread_self(), dur, pl, CLOSD, true, NOFD);
            bathroomOpen = false;
            break;
        default:
            break;
    }

    free(op);
}

int createAndOpenPrivateFIFO(char * private_fifoname) {
    int privatefd;
    sprintf(private_fifoname, "/tmp/%d.%lu", getpid(), pthread_self());
    if (mkfifo(private_fifoname, 0660) == -1) {
        write(STDERR_FILENO, "Error making fifo", 17);
        pthread_exit(NULL);
    }

    if ((privatefd = open(private_fifoname, O_RDONLY | O_NONBLOCK)) == -1){
        write(STDERR_FILENO, "Error opening private fifo", 26);
        pthread_exit(NULL);
    }
    return privatefd;
}

int openServerFIFO(int n, int dur) {
    int fd;
    if((fd = open(fifoname, O_WRONLY)) == -1){
        logOperation(n, getpid(), pthread_self(), dur, -1, FAILD, true, NOFD);
        pthread_exit(NULL);
    }
    return fd;
}

void * sendRequest(void *args){
    int n = *(int *) args;
    int fd;
    int dur = 50 + rand()%50;

    fd = openServerFIFO(n, dur);

    char * private_fifoname = malloc(MAX_STRING_SIZE);
    int privatefd = createAndOpenPrivateFIFO(private_fifoname);

    logOperation(n, getpid(), pthread_self(), dur, -1, IWANT, true, fd);
    close(fd);

    waitResponse(privatefd);

    close(privatefd);
    unlink(private_fifoname);
    free(private_fifoname);
    pthread_exit(NULL);
}


int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    srand(time(NULL));
    int count = 0;
    while((clock_gettime(CLOCK_MONOTONIC_RAW, &end), (end.tv_sec + (end.tv_nsec/(1000000000.))) - (start.tv_sec + (start.tv_nsec/(1000000000.))) < nsecs && bathroomOpen)) {
        pthread_t thread;
        int n = count;
        pthread_create(&thread, NULL, sendRequest, &n);
        unsigned msInterval = 2 + rand() % 5;
        usleep(msInterval*1000); // sleeps a random number of milliseconds
        count++;
    }
    
    pthread_exit(EXIT_SUCCESS);
}

