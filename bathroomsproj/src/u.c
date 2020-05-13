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
int nsecs, publicfd;
char * fifoname;
bool bathroomOpen = true, running = true;


void setArgs(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
    alarm(nsecs);
}

void sig_handler(int signo) {
    if (signo == SIGALRM) {
        running = false;
    }
}

void waitResponse(int privatefd){
    structOp *op = malloc(sizeof(structOp));
    int i, dur, pl;
    pid_t pid;
    pthread_t tid;
    enum OPERATION oper;

    int n;

    while((n = read(privatefd, op, sizeof(structOp))) <= 0) {
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
        perror("Error making private fifo");
        pthread_exit(NULL);
    }

    if ((privatefd = open(private_fifoname, O_RDONLY | O_NONBLOCK)) == -1) {
        perror("Error opening private fifo");
        pthread_exit(NULL);
    }

    return privatefd;
}

void openServerFIFO() {
    if ((publicfd = open(fifoname, O_WRONLY)) == -1){
        perror("Error opening public fifo");
        pthread_exit(NULL);
    }
}

void * sendRequest(void *args){
    int n = *(int *) args;
    int dur = 50 + rand()%50;

    char * private_fifoname = malloc(MAX_STRING_SIZE);
    int privatefd = createAndOpenPrivateFIFO(private_fifoname);

    logOperation(n, getpid(), pthread_self(), dur, -1, IWANT, true, publicfd);

    waitResponse(privatefd);

    close(privatefd);
    unlink(private_fifoname);
    free(private_fifoname);
    pthread_exit(NULL);
}


int main(int argc, char ** argv) {
    struct sigaction action;

    action.sa_handler = &sig_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    sigaction(SIGALRM, &action, NULL);

    setArgs(argc, argv);
    openServerFIFO();

    srand(time(NULL));

    int count = 0;
    int numbers[100];
    while(running && bathroomOpen) {
        pthread_t thread;
        numbers[count % 100] = count;
        pthread_create(&thread, NULL, sendRequest, &numbers[count % 100]);
        unsigned msInterval = 10 + rand() % 5;
        usleep(msInterval*1000); // sleeps a random number of milliseconds
        count++;
    }
    
    close(publicfd);
    pthread_exit(EXIT_SUCCESS);
}

