#include <stdio.h>
#include <stdlib.h>
#include "cmdargs.h"

#define MAX_STRING_SIZE 512
int nsecs;
char * fifoname;

void setArgs(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;
}

int main(int argc, char ** argv) {
    setArgs(argc, argv);
    
    exit(EXIT_SUCCESS);
}

