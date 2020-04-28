struct QArgs_struct {
    int nsecs, nplaces, nthreads;
    char  * fifoname;
};
typedef struct QArgs_struct QArgs;

struct UArgs_struct {
    int nsecs;
    char  * fifoname;
};
typedef struct UArgs_struct UArgs;


/**
 *  Gets the commandLineArgs for Q 
 */
QArgs getCommandLineArgsQ(int argc, char * argv[]);

/**
 *  Gets the commansLineArgs for U
 */
UArgs getCommandLineArgsU(int argc, char * argv[]);
