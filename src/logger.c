#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define MAX_STRING_SIZE 1024

extern char * logFilename;
char str[MAX_STRING_SIZE];
int fd;

char * eventStrings[7] = {"CREATE", "EXIT", "RECV_SIGNAL", "SEND_SIGNAL", "RECV_PIPE", "SEND_PIPE", "ENTRY"};

void clearLogFile() {
    close(open(logFilename, O_WRONLY | O_TRUNC | O_CREAT, 0644));
    fd = open(logFilename, O_WRONLY | O_APPEND);
}

void logMessage(char * message) {
    if (fd == -1) {
        printf("Error opening log file.\n");
        exit(1);
    }
    sprintf(str, "%s", message);
    if (write(fd, str, strlen(str)) < 0) {
        printf("Error writing to log file.\n");
        exit(1);
    }
}

void closeLog(){
    close(fd);
}

void logEVENT(enum EVENT event, double instant, int pid, char * info) {
    char * message = malloc(MAX_STRING_SIZE);
    sprintf(message, "%.2lf - %-8d - %s - %s\n", instant, pid, eventStrings[event], info);
    logMessage(message);
}

