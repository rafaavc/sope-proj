#include <stdio.h>
#include <stdlib.h>
#include "cmdargs.h"

#define MAX_STRING_SIZE 512
int nsecs;
char * fifoname;

int main(int argc, char ** argv) {
    UArgs args = getCommandLineArgsU(argc, argv);
    nsecs = args.nsecs;
    fifoname = args.fifoname;

    if (nsecs != -1) printf("nsecs: %d\n", nsecs);
    printf("fifoname: %s\n", fifoname);

    exit(EXIT_SUCCESS);
}

