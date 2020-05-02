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

void sigint_handler(int sig){
    if (sig == SIGUSR1){
        bathroomOpen = false;
    }
}

void setArgs(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
}


void waitResponse(int privatefd){
    char *string = malloc(MAX_STRING_SIZE);
    int i, pid, dur, pl;
    long t, tid;
    enum OPERATION oper;

    int n;

    while((n = read(privatefd, string, MAX_STRING_SIZE)) <= 0 /*&& timeElapsed < nsecs*/) {
        if (n == -1) {
            if (errno != EAGAIN) {
                perror("Error reading from private fifo");
                return;
            }
        }
    }

    receiveLogOperation(&string[0], &t, &i, &pid, &tid, &dur, &pl, &oper);
    
    switch(oper) {
        case ENTER:
            logOperation(i, pid, tid, dur, pl, IAMIN, 1, STDOUT_FILENO);
            break;
        case TLATE:
            bathroomOpen = false;
            logOperation(i, getpid(), pthread_self(), dur, pl, CLOSD, 1, STDOUT_FILENO);
            break;
        
        default:
            break;
    }

    free(string);
}

void * sendRequest(void *args){
    struct timespec t;
    int n = *(int *) args;
    int fd;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    int dur = 20 + rand() % 20;

    if (!bathroomOpen){
            logOperation(n, getpid(), pthread_self(), dur, -1, CLOSD, 1, STDOUT_FILENO);
            pthread_exit(NULL);
    }

    if((fd = open(fifoname, O_WRONLY)) == -1){
        logOperation(n, getpid(), pthread_self(), dur, -1, FAILD, 1, STDOUT_FILENO);
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

    logOperation(n, getpid(), pthread_self(), dur, -1, IWANT, 2, STDOUT_FILENO, fd);
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

    struct sigaction action;

    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGUSR1, &action, NULL);

    srand(time(NULL));
    int count = 0;
    while((clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs)) {
        pthread_t thread;
        pthread_create(&thread, NULL, sendRequest, (void *) &count);
        unsigned msInterval = 100 + rand()%80;
        usleep(msInterval*1000); // sleeps a random number of milliseconds (from 100 to 180)
        count++;
    }

    pthread_exit(EXIT_SUCCESS);
}

