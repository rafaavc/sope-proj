#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "cmdargs.h"
#include <string.h>

#define MAX_STRING_SIZE 512
#define MAX_NUMBER_THREADS 100
int nsecs;
char * fifoname;
bool bathroomOpen = true;
int fd;


void setArgs(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
}

void * sendRequest(void *args){
    struct timespec t;
    int n = *(int *) args;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    srand(time(NULL));
    int dur = rand() % 20;
    char *string = malloc(MAX_STRING_SIZE);
    sprintf(string, "%ld; %d; %d; %ld; %d; %d; IWANT\n", t.tv_sec, n, getpid(), pthread_self(), dur, -1);
    write(fd, string, strlen(string));
    printf("%s", string);
    return NULL;
}


int main(int argc, char ** argv) {
    setArgs(argc, argv);
    pthread_t threads[MAX_NUMBER_THREADS];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
    while((fd = open(fifoname, O_WRONLY)) <= 0){
        sleep(1);
    }

    srand(time(0));
    int count = 0;
    while(bathroomOpen && clock_gettime(CLOCK_MONOTONIC_RAW, &end), end.tv_sec - start.tv_sec < nsecs && count < MAX_NUMBER_THREADS) {
        pthread_create(&threads[count], NULL, sendRequest, (void *) &count);
        unsigned msInterval = 80 + (((float)rand()/RAND_MAX)*80);
        usleep(msInterval*1000); // sleeps a random number of milliseconds (from 80 to 160)
        count++;
    }

    exit(EXIT_SUCCESS);
}

