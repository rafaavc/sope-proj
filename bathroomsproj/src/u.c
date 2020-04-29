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

#define MAX_STRING_SIZE 512
int nsecs;
char * fifoname;
bool bathroomOpen = true;
int fd;


void setArgs(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
}


void waitResponse(){
    int privatefd, numbers[6];
    char *private_fifoname = malloc(MAX_STRING_SIZE);
    sprintf(private_fifoname, "/tmp/%d.%ld", getpid(), pthread_self());
    mkfifo(private_fifoname, 0644);

    if ((privatefd = open(private_fifoname, O_RDONLY)) <= 0){
        printf("Error opening private fifo");
        exit(1);
    }

    char *string = malloc(MAX_STRING_SIZE);
    int i, pid, dur, pl;
    long t, tid;
    enum OPERATION oper;

    read(privatefd, string, MAX_STRING_SIZE);

    receiveLogOperation(&string[0], &t, &i, &pid, &tid, &dur, &pl, &oper);
    oper = IAMIN;
    logOperation(i, pid, tid, dur, pl, oper, STDOUT_FILENO);

}

void * sendRequest(void *args){
    struct timespec t;
    int n = *(int *) args;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    srand(time(NULL));
    int dur = rand() % 20;
    char *string = logOperation(n, getpid(), pthread_self(), dur, -1, IWANT, STDOUT_FILENO);
    write(fd, string, strlen(string));

    waitResponse();

    return NULL;
}


int main(int argc, char ** argv) {
    setArgs(argc, argv);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    while((fd = open(fifoname, O_WRONLY)) <= 0){
        sleep(1);
    }

    srand(time(0));
    int count = 0;
    while(bathroomOpen && (clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs)) {
        pthread_t thread;
        pthread_create(&thread, NULL, sendRequest, (void *) &count);
        unsigned msInterval = 80 + (((float)rand()/RAND_MAX)*80);
        usleep(msInterval*1000); // sleeps a random number of milliseconds (from 80 to 160)
        count++;
    }

    close(fd);
    exit(EXIT_SUCCESS);
}

