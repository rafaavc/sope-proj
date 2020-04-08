#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_STRING_SIZE 1024

extern char * logFilename;
char str[MAX_STRING_SIZE];
int fd;
struct timespec start;

char * eventStrings[7] = {"CREATE", "EXIT", "RECV_SIGNAL", "SEND_SIGNAL", "RECV_PIPE", "SEND_PIPE", "ENTRY"};

void openLogFile() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    close(open(logFilename, O_WRONLY | O_TRUNC | O_CREAT, 0644));
    fd = open(logFilename, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("Error opening log file");
        terminateProcess(EXIT_FAILURE);
    }
}

void logMessage(char * message) {
    sprintf(str, "%s", message);
    if (write(fd, str, strlen(str)) < 0) {
        perror("Error writing to log file");
        terminateProcess(EXIT_FAILURE);
    }
}

void closeLog(){
    close(fd);
}

void logEVENT(enum EVENT event, int pid, char * info) {
    char * message = malloc(MAX_STRING_SIZE);

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    double instant = (end.tv_sec - start.tv_sec) * 1000. + ((end.tv_nsec - start.tv_nsec) / 1000000.);

    sprintf(message, "%.2lf - %-8d - %s - %s\n", instant, pid, eventStrings[event], info);
    logMessage(message);
}

void terminateProcess(int status) {
    char statuss[2];
    sprintf(statuss, "%d", status);
    logEVENT(EXIT, getpid(), statuss);
    closeLog();
    exit(status);
}

char * satos(char ** sa, int c) {
    char * res = malloc(MAX_STRING_SIZE);
    sprintf(res, "%s", sa[0]);
    for (int i = 1; i < c; i++) {
        sprintf(res, "%s, %s", res, sa[i]);
    }
    return res;
}
