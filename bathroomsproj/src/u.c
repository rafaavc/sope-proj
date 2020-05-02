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
            logOperation(i, pid, tid, dur, pl, IAMIN, true, -1);
            break;
        case TLATE:
            logOperation(i, getpid(), pthread_self(), dur, pl, CLOSD, true, -1);
            break;
        default:
            break;
    }

    free(op);
}

void * sendRequest(void *args){
    struct timespec t;
    int n = *(int *) args;
    int fd;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    int dur = 150 + rand() % 150;

    if((fd = open(fifoname, O_WRONLY)) == -1){
        logOperation(n, getpid(), pthread_self(), dur, -1, FAILD, true, -1);
        pthread_exit(NULL);
    }

    int privatefd;
    char *private_fifoname = malloc(MAX_STRING_SIZE);
    sprintf(private_fifoname, "/tmp/%d.%ld", getpid(), pthread_self());
    mkfifo(private_fifoname, 0660);

    if ((privatefd = open(private_fifoname, O_RDONLY | O_NONBLOCK)) == -1){
        write(STDOUT_FILENO, "Error opening private fifo", 26);
        pthread_exit(NULL);
    }

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
    while((clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs)) {
        pthread_t thread;
        pthread_create(&thread, NULL, sendRequest, &count);
        unsigned msInterval = 2 + rand() % 5;
        usleep(msInterval*1000); // sleeps a random number of milliseconds
        count++;
    }

    pthread_exit(EXIT_SUCCESS);
}

