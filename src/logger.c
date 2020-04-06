#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING_SIZE 1024

extern char * logFilename;

char * eventStrings[7] = {"CREATE", "EXIT", "RECV_SIGNAL", "SEND_SIGNAL", "RECV_PIPE", "SEND_PIPE", "ENTRY"};

void clearLogFile() {
    fclose(fopen(logFilename, "w"));
}

void logMessage(char * message) {
    FILE * file = fopen(logFilename, "a");
    if (file == NULL) {
        printf("Error opening log file.\n");
        exit(1);
    }
    if (fprintf(file, "%s\n", message) < 0) {
        printf("Error writing to log file.\n");
        exit(1);
    }
    fclose(file);
}

void logEVENT(enum EVENT event, double instant, int pid, char * info) {
    char * message = malloc(MAX_STRING_SIZE);
    sprintf(message, "%.2lf - %-8d - %s - %s", instant, pid, eventStrings[event], info);
    logMessage(message);
}

